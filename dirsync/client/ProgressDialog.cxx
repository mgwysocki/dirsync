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
