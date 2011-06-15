#include <QTest>
#include <QUdpSocket>

#include "common/network_base.hpp"


void processPhysics () {

}

void recieveData (QUdpSocket& socket) {
  while (socket.hasPendingDatagrams ()) {
//     ... socket.pendingDatagramSize ();
//     if (socket.readDatagram ()) {
//       ...
//     }
  }
}

void sendData () {

}


int main () {
  QUdpSocket socket;
  socket.bind (DEFAULT_NETWORK_PORT, QUdpSocket::DontShareAddress | QUdpSocket::ReuseAddressHint);
  while (true) {
    recieveData (socket);
    processPhysics ();
    sendData ();
    QTest::qWait (1);
  }
  return 0;
}
