
#include <iostream>
using namespace std;


class FileObject
{
 public:
  FileObject();
  FileObject(const FileObject &);

  void begin_receive_file(const QString &filename);

 public slots:
  void receive_payload();
  
 signals:
  void finished();

 private:
  void end_receive_file();
  QString _filename;
  QFile _file;
  QByteArray _buffer;
}
