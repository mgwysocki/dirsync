#ifndef __PROGRESSDIALOG_H
#define __PROGRESSDIALOG_H

#include <QDialog>
#include <QProgressBar>
#include <QLabel>
#include <iostream>

class QPushButton;

class ProgressDialog : public QDialog
{
Q_OBJECT
 public:
  ProgressDialog(QWidget* parent=0);
  
  void set_n_upload(qint64 n);
  void set_n_download(qint64 n);

 public slots:
  void done();

  void set_upload_status(QString text) {_upload_status.setText(text);}
  void set_download_status(QString text) {_download_status.setText(text);}
  void increment_upload() {_upload_bar.setValue( _upload_bar.value()+1 );}
  void increment_download() {_download_bar.setValue( _download_bar.value()+1 );}

  void increment_upload(qint64 n) {
    _total_uploaded += n;
    _upload_bar.setValue( int(_total_uploaded*100/_total_upload) );
  }

  void increment_download(qint64 n) {
    _total_downloaded += n;
    _download_bar.setValue( int(_total_downloaded*100/_total_download) );
  }

 private:
  QProgressBar _upload_bar;
  QProgressBar _download_bar;
  QLabel _upload_status;
  QLabel _download_status;

  QPushButton* _button;

  qint64 _total_upload;
  qint64 _total_uploaded;
  qint64 _total_download;
  qint64 _total_downloaded;
};

#endif
