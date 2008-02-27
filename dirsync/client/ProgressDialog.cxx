#include "ProgressDialog.h"

#include <QVBoxLayout>
#include <QPushButton>

ProgressDialog::ProgressDialog(QWidget* parent) :
  QDialog(parent)
{
  setModal(true);
  setSizeGripEnabled(true);

  QPushButton* cancel_button = new QPushButton("Cancel", this);
  connect(cancel_button, SIGNAL(clicked()), this, SLOT(reject()));

  QVBoxLayout* layout = new QVBoxLayout;
  layout->addWidget(&_upload_bar);
  layout->addWidget(&_upload_status);
  layout->addWidget(&_download_bar);
  layout->addWidget(&_download_status);
  layout->addWidget(cancel_button);
  setLayout(layout);

  setWindowTitle( tr("Sync Progress") );
}

void ProgressDialog::set_n_upload(int n) 
{
  if(n>0) {
    _upload_bar.setRange(0, n);
    _upload_bar.setValue(0);
  } else {
    _upload_bar.setRange(0, 1);
    _upload_bar.setValue(1);
  }
  return;
}

void ProgressDialog::set_n_download(int n) 
{
  if(n>0) {
    _download_bar.setRange(0, n);
    _download_bar.setValue(0);
  } else {
    _download_bar.setRange(0, 1);
    _download_bar.setValue(1);
  }
  return;
}
