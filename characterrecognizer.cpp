#include "characterrecognizer.h"

#include <QImage>
#include <QDebug>
#include <QFile>

#include <sstream>

#include <dlib/dnn.h>
#include <dlib/data_io.h>

using dlib::relu;
using dlib::con;
using dlib::max_pool;
using dlib::inception3;
using dlib::inception4;
using dlib::fc;
using dlib::loss_multiclass_log;
template <typename SUBNET> using block_a1 = relu<con<10,1,1,1,1,SUBNET>>;
template <typename SUBNET> using block_a2 = relu<con<10,3,3,1,1,relu<con<16,1,1,1,1,SUBNET>>>>;
template <typename SUBNET> using block_a3 = relu<con<10,5,5,1,1,relu<con<16,1,1,1,1,SUBNET>>>>;
template <typename SUBNET> using block_a4 = relu<con<10,1,1,1,1,max_pool<3,3,1,1,SUBNET>>>;

// Here is inception layer definition. It uses different blocks to process input
// and returns combined output.  Dlib includes a number of these inceptionN
// layer types which are themselves created using concat layers.
template <typename SUBNET> using incept_a = inception4<block_a1,block_a2,block_a3,block_a4, SUBNET>;

// Network can have inception layers of different structure.  It will work
// properly so long as all the sub-blocks inside a particular inception block
// output tensors with the same number of rows and columns.
template <typename SUBNET> using block_b1 = relu<con<4,1,1,1,1,SUBNET>>;
template <typename SUBNET> using block_b2 = relu<con<4,3,3,1,1,SUBNET>>;
template <typename SUBNET> using block_b3 = relu<con<4,1,1,1,1,max_pool<3,3,1,1,SUBNET>>>;
template <typename SUBNET> using incept_b = inception3<block_b1,block_b2,block_b3,SUBNET>;

// Now we can define a simple network for classifying MNIST digits.  We will
// train and test this network in the code below.
using net_type = loss_multiclass_log<
        fc<26,
        relu<fc<32,
        max_pool<2,2,2,2,incept_b<
        max_pool<2,2,2,2,incept_a<
        dlib::input<dlib::matrix<unsigned char>>
        >>>>>>>>;

//struct Network : public net_type { } *CharacterRecognizer::m_net;
struct Network
{
    net_type net;
};

CharacterRecognizer *CharacterRecognizer::instance()
{
    static CharacterRecognizer inst;
    return &inst;
}

QString CharacterRecognizer::recognize(const QImage &image)
{
    if (image.width() != 28 || image.height() != 28) {
        qWarning() << "Invalid image geometry" << image.size();
        return "!";
    }
    if (image.format() != QImage::Format_Grayscale8) {
        qWarning() << "Invalid format" << image.format();
        return "-";
    }

    dlib::matrix<unsigned char> dlibImage(28, 28);
//    dlib::matrix<unsigned char, 28, 28> dlibImage;
    for (int y=0; y<image.height(); y++) {
        const uchar *line = image.scanLine(y);
        for (int x=0; x<image.width(); x++) {
            dlibImage(x, y) = line[x];
        }
    }

    const unsigned long pred = m_net->net(dlibImage);
    qDebug() << pred;
//    std::vector<dlib::matrix<
//    std::vector<dlib::matrix<unsigned char>> testing_images;
//    testing_images.push_back(dlibImage);
//    std::vector<unsigned long> pred = m_net->net(testing_images);
//    std::vector<long> pred = (*m_net)(dlibImage);
//    for (const long &f : pred) {
//        qDebug() << f;
//    }

    return QString(QChar(ushort(pred + 'a')));
}

CharacterRecognizer::CharacterRecognizer(QObject *parent) : QObject(parent)
{
    m_net = new Network;
    QFile netFile(":/net.dat");
    if (!netFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to load net file";
        return;
    }
    std::string netData = netFile.readAll().toStdString();
    std::istringstream istr(netData);
    dlib::deserialize(m_net->net, istr);
    qDebug() << "Network ready";
}
