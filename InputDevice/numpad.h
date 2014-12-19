#ifndef NUMPAD_H
#define NUMPAD_H
#include <QtDesigner/QDesignerExportWidget>
#include <QDialog>
#include <QVariant>
#include <QLineEdit>

namespace Ui {
    class Numpad;
}

class Numpad : public QWidget
{
    Q_OBJECT

public:
    explicit Numpad(QWidget *parent = 0);
    ~Numpad();

    void activate(QVariant initValue,const QValidator *validator);
    void setEchoMode(QLineEdit::EchoMode);
private slots:
    void buttonClicked();
    void enterClicked();
    void escapeClicked();
    void clearClicked();
    void backspaceClicked();
    void periodClicked();
    void pmClicked();
    void saveFocusWidget(QWidget * /*oldFocus*/, QWidget *newFocus);
private:
    Ui::Numpad *ui;
    QObjectList keyList;
    const QValidator *validator;

    bool bFirst;
    float min_,max_;
    void initKeys();
    void popData();
    void checkValue();
    void checkFirst();
    QWidget *lastFocusedWidget;

    QPalette orgPalette;
protected:
    bool event(QEvent *e);
signals:
    void dataSet(QVariant *);
};

#endif // NUMPAD_H
