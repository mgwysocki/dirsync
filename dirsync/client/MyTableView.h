#ifndef __MYTABLEVIEW_H
#define __MYTABLEVIEW_H

#include <QTableView>
#include <QResizeEvent>

#include <iostream>
using namespace std;

class MyTableView : public QTableView
{
  Q_OBJECT
 public:
  MyTableView(QWidget* parent=0) : QTableView::QTableView(parent) {}

  void resizeEvent(QResizeEvent* event) {
    QTableView::resizeEvent(event);

    int total_width = viewport()->width();
    int min0 = sizeHintForColumn(0);
    int min1 = sizeHintForColumn(1);
    int min2 = sizeHintForColumn(2);
    int min3 = sizeHintForColumn(3);

    setColumnWidth(1, min1);
    setColumnWidth(2, min2);
   
    if(min0+min1+min2+min3<total_width) {
      int size3 = (total_width - min1 - min2)/2;
      int size0 = total_width - min1 - min2 - size3;
      setColumnWidth(0, size0);
      setColumnWidth(3, size3);

    } else {
      setColumnWidth(0, min0);
      setColumnWidth(3, min3);
    }
    return;
  }

};

#endif //__MYTABLEVIEW_H
