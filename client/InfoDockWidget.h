#ifndef __INFODOCKWIDGET_H
#define __INFODOCKWIDGET_H

#include <QDockWidget>
class QTextEdit;
class QSplitter;

class InfoDockWidget : public QDockWidget
{
Q_OBJECT
 public:
  InfoDockWidget(QWidget* parent = 0);

 public slots:
  void clear_info();
  void set_info(QString, QString);
  void set_client_info(QString);
  void set_server_info(QString);

 private:
  QTextEdit* _client_textedit;
  QTextEdit* _server_textedit;
  QSplitter* _splitter;
};

#endif // __INFODOCKWIDGET_H
