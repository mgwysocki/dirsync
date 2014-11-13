#ifndef __PREFERENCES_DIALOG_H
#define __PREFERENCES_DIALOG_H

#include <QDialog>
#include <QLineEdit>

class QDialogButtonBox;
class QLabel;
class QPushButton;
class QCheckBox;

#include "../Preferences.h"

class PreferencesDialog : public QDialog
{
Q_OBJECT

 public:
  PreferencesDialog(QWidget *parent = 0);

 private slots:
  void _set_preferences();

 private:
  Preferences* _prefs;

  QLabel* _tempdir_label;
  QLineEdit* _tempdir_lineedit;
  QCheckBox* _perms_checkbox;

  QPushButton* _okay_button;
  QPushButton* _apply_button;
  QPushButton* _cancel_button;
  QDialogButtonBox* buttonBox;
};

#endif
