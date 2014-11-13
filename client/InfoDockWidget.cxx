
#include "InfoDockWidget.h"

#include <QTextEdit>
#include <QSplitter>

InfoDockWidget::InfoDockWidget(QWidget* parent) :
  QDockWidget::QDockWidget( tr("File Info"), parent ),
  _client_textedit( new QTextEdit(this) ),
  _server_textedit( new QTextEdit(this) ),
  _splitter( new QSplitter(this) )
{
  _client_textedit->setTextInteractionFlags(Qt::TextSelectableByMouse|Qt::TextSelectableByKeyboard);
  _server_textedit->setTextInteractionFlags(Qt::TextSelectableByMouse|Qt::TextSelectableByKeyboard);
  _splitter->insertWidget(0, _client_textedit);
  _splitter->insertWidget(1, _server_textedit);
  this->setWidget(_splitter);
}

void InfoDockWidget::clear_info()
{
  _client_textedit->clear();
  _server_textedit->clear();
}

void InfoDockWidget::set_info(QString client_info, QString server_info)
{
  _client_textedit->setHtml(client_info);
  _server_textedit->setHtml(server_info);
}

void InfoDockWidget::set_client_info(QString info)
{_client_textedit->setHtml(info);}

void InfoDockWidget::set_server_info(QString info)
{_server_textedit->setHtml(info);}
