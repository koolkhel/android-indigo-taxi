#ifndef CUSTOMEDIT_H
#define CUSTOMEDIT_H

#endif // CUSTOMEDIT_H

#include <QLineEdit>


class KeyboardLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    KeyboardLineEdit(QWidget *parent=0) :
                     QLineEdit(parent)
    {

    }

};
