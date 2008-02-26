#ifndef __PROGRESSDIALOG_H
#define __PROGRESSDIALOG_H

#include <QDialog>
#include <QProgressBar>
#include <QLabel>

class ProgressDialog : public QDialog
{
Q_OBJECT
 public:
  ProgressDialog(QWidget* parent=0);
  
  void set_n_upload(int n) {_upload_bar.setRange(0, n);}
  void set_n_download(int n) {_download_bar.setRange(0, n);}

 public slots:
  void set_upload_status(QString text) {_upload_status.setText(text);}
  void set_download_status(QString text) {_download_status.setText(text);}
  void increment_upload() {_upload_bar.setValue( _upload_bar.value()+1 );}
  void increment_download() {_download_bar.setValue( _download_bar.value()+1 );}

 private:
  QProgressBar _upload_bar;
  QProgressBar _download_bar;
  QLabel _upload_status;
  QLabel _download_status;
};

#endif
