#ifndef C_IMAGE_H
#define C_IMAGE_H

#include <QWidget>
#include <ui/c_widget.h>
#include <QPropertyAnimation>
#include "utils/utils.h"

namespace Ui {
class c_image;
}

class c_image : public c_widget
{
    Q_OBJECT

public:

    explicit c_image(QString pathImage = QString(), c_widget *widget = nullptr, QWidget *parent = nullptr);
    ~c_image() override;

    QPixmap getImage() const;
    QSize getSize(modes target = modes::display) const override;
    void setImage(const QPixmap &value);

    void save() override;
    void rollback() override;

    bool isEmpty() const;
    QAbstractAnimation *switchMode(modes target = modes::resume, bool animated = true, int time = 600, QAnimationGroup *parentGroupAnimation = nullptr);

    QString getPathImage() const;

public slots:
    void addButtonClicked();
    void deleteButtonClicked();

signals:
    void newImage();
    void resized();

private:

    Ui::c_image *ui;

    QMap<int,QSize> imageSizes;
    QMap<int,QSize> addButtonSizes;

    QString pathImage;
    QString pathOldImage;
    QPixmap image;
    QPixmap oldImage;

    QSize deleteButtonSize;
};

#endif // C_IMAGE_H
