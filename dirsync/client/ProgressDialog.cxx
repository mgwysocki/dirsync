#include "ProgressDialog.h"

#include <QVBoxLayout>
#include <QPushButton>

ProgressDialog::ProgressDialog(QWidget* parent) :
  QDialog(parent),
  _total_upload(0),
  _total_uploaded(0),
  _total_download(0),
  _total_downloaded(0)
{
  setModal(true);
  setSizeGripEnabled(true);

  _button = new QPushButton("Cancel", this);
  connect(_button, SIGNAL(clicked()), this, SLOT(reject()));

  QVBoxLayout* layout = new QVBoxLayout;
  layout->addWidget(&_upload_bar);
  layout->addWidget(&_upload_status);
  layout->addWidget(&_download_bar);
  layout->addWidget(&_download_status);
  layout->addWidget(_button);
  setLayout(layout);

  setWindowTitle( tr("Sync Progress") );
}

void ProgressDialog::done()
{
  _button->setText("Close");
}

void ProgressDialog::set_n_upload(qint64 n) 
{
  if(n>0) {
    _upload_bar.setRange(0, 100);
    _total_upload = n;
    _upload_bar.setValue(0);
  } else {
    _upload_bar.setRange(0, 1);
    _upload_bar.setValue(1);
  }
  return;
}

void ProgressDialog::set_n_download(qint64 n) 
{
  if(n>0) {
    _download_bar.setRange(0, 100);
    _total_download = n;
    _download_bar.setValue(0);
  } else {
    _download_bar.setRange(0, 1);
    _download_bar.setValue(1);
  }
  return;
}
