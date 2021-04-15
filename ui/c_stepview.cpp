#include "c_stepview.h"
#include "ui_c_step_view.h"

QSize c_stepView::maxSizeImage(150,150);
int c_stepView::interImageSpace = 5;
int c_stepView::borderSize = 9;
int c_stepView::showButtonHeight = 21;
int c_stepView::borderMenuButton = 6;
int c_stepView::maxNumberImages = 4;

c_stepView::c_stepView(c_step *_step, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::c_stepView), step(_step) {
    ui->setupUi(this);

    QRect rect(2,2,46,46);
    QRegion region(rect, QRegion::Ellipse);
    ui->rankButton->setMask(region);
    QObject::connect(ui->showButton,&QPushButton::released,this,&c_stepView::slot_triggerShowImages);
    showImage = false;

    ui->label->append(step->getDescription());
    ui->label->setAlignment(Qt::AlignJustify);

    ui->rankButton->setText(QString("%1").arg(step->getRank()));
    rankEdit = step->getRank();

    ui->saveButton->setFixedWidth(ui->rankButton->width());
    ui->saveButton->setFixedSize(ui->saveButton->size());
    ui->saveButton->move(QPoint(-ui->saveButton->width(),0));
    ui->cancelButton->setFixedWidth(ui->rankButton->width());
    ui->cancelButton->setFixedSize(ui->cancelButton->size());
    ui->cancelButton->move(QPoint(-ui->cancelButton->width(),0));
    ui->upButton->setFixedSize(ui->upButton->size());
    ui->upButton->move(QPoint(this->width(),0));
    ui->downButton->setFixedSize(ui->downButton->size());
    ui->downButton->move(QPoint(this->width(),0));

    ui->newImageButton->hide();

    QList<QString> imageStringList = step->getImagesUrl();
    for (QList<QString>::iterator it = imageStringList.begin(); it != imageStringList.end() ; ++it ) {
        imageList.push_back(QPixmap(*it));
        imageSlots.push_back(new QLabel(this));
    }

    QMenu *menu = new QMenu();
    menu->addAction("Edit",this,&c_stepView::editStepAnimationOn);
    menu->addAction("Delete");
    menu->addAction("Add note");

    ui->menuButton->setMenu(menu);

    ui->rankButton->move(borderSize,borderSize);

    this->setBackgroundRole(QPalette::Background);

    state = states::retracted;
    imageNumberChanged = false;
}

c_stepView::~c_stepView() {
    delete ui;
}

void c_stepView::setRank(int rank) {
    rankEdit = rank;
    step->setRank(rank);
}

void c_stepView::slot_triggerShowImages() {
    lockSize(false);
    ui->showButton->raise();
    switch (state) {
        case states::retracted:
            state = states::transition;
            for (int i = 0; i < imageSlots.size(); ++i) {
                imageSlots[i]->show();
            }
            openImageSlot();
            break;
        case states::opened:
            state = states::transition;
            closeImageSlot();
            break;
        default:
            break;
    }
}

void c_stepView::resizeEvent(QResizeEvent */*event*/) {
    QPixmap image;
    switch (state) {
        case states::retracted : {
            this->setFixedWidth(this->size().width());

            // label
            QFontMetrics metrics(ui->label->document()->firstBlock().charFormat().font());
            ui->label->setFixedWidth(this->width() - ui->rankButton->width() - 2*borderSize - ui->menuButton->width()-2*borderMenuButton);
            ui->label->setFixedHeight(getHeightText()+8);
            ui->label->move(ui->rankButton->width()+borderSize*2,borderSize);
            ui->label->setReadOnly(true);

            // rank button
            ui->rankButton->move(borderSize,borderSize);

            // show button
            ui->showButton->setFixedSize(this->width()-2*borderSize,showButtonHeight);
            ui->showButton->move(borderSize,borderSize*2+std::max(ui->rankButton->height(),ui->label->height()));

            // menu button
            ui->menuButton->move(this->width()-borderMenuButton-ui->menuButton->width(),borderSize);

            int heightMin = borderSize*2 + std::max(ui->rankButton->height(),ui->label->height()) + ui->showButton->height();
            this->setFixedHeight(heightMin);

            // format images
            int maxW = (this->size().width() - (imageSlots.size()-1)*interImageSpace - 2*borderSize)/imageSlots.size();

            maxW = maxW>maxSizeImage.width()?maxSizeImage.width():maxW;
            QSize labelsize(maxW>maxSizeImage.width()?maxSizeImage.width():maxW,maxSizeImage.height());
            for (int i = 0; i < imageSlots.size(); ++i) {
                image = imageList[i].scaled(labelsize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                imageSlots[i]->setFixedHeight(image.size().height());
                imageSlots[i]->setFixedWidth(image.size().width());
                imageSlots[i]->setPixmap(image);
                imageSlots[i]->setBackgroundRole(QPalette::Base);
                imageSlots[i]->setScaledContents(true);
                imageSlots[i]->hide();
            }
            QList<QPoint> posList = arrangeImages();
            for (int i = 0; i < imageSlots.size(); ++i) {
                imageSlots[i]->move(posList[i]);
            }
            ui->widgetButton->raise();
            break;
        }
        case states::opened : {
            lockSize(true);
            break;
        }
        case states::transition:

            break;
    }
}

void c_stepView::openImageSlot() {
    QParallelAnimationGroup *group = new QParallelAnimationGroup;
    int hMax = getImagesMaxHeigth();
    group->addAnimation(growAnimation(this,QSize(0,hMax+borderSize)));
    group->addAnimation(slideAnimation(ui->showButton,QPoint(0,hMax+borderSize)));

    for (int i = 0; i < imageSlots.size(); ++i) {
        group->addAnimation(fadeAnimation(imageSlots[i],true));
    }

    QObject::connect(group,&QParallelAnimationGroup::finished,[=] () {endTransition(states::opened);});
    group->start(QAbstractAnimation::DeleteWhenStopped);
}

void c_stepView::closeImageSlot() {
    QParallelAnimationGroup *group = new QParallelAnimationGroup;
    int hMax = getImagesMaxHeigth();
    group->addAnimation(growAnimation(this,QSize(0,-(hMax+borderSize))));
    group->addAnimation(slideAnimation(ui->showButton,QPoint(0,-(hMax+borderSize))));

    for (int i = 0; i < imageSlots.size(); ++i) {
        group->addAnimation(fadeAnimation(imageSlots[i],false));
    }

    QObject::connect(group,&QParallelAnimationGroup::finished,[=] () {endTransition(states::retracted);});
    state = states::transition;
    group->start(QAbstractAnimation::DeleteWhenStopped);
}

void c_stepView::editStepAnimationOn() {
    int slideDistance = 0;
    int finalLabelHeight = ui->label->height();
    int interButton = 0;
    QParallelAnimationGroup *group = new QParallelAnimationGroup;
    QRect rect;

    lockSize(false);

    ui->menuButton->setDisabled(true);

    ui->label->setReadOnly(false);
    ui->label->setStyleSheet("");
    ui->label->raise();

    QFontMetrics metrics(ui->label->document()->firstBlock().charFormat().font());
    ui->label->setFixedSize(QSize(QWIDGETSIZE_MAX,QWIDGETSIZE_MAX));
    ui->label->setFixedWidth(ui->label->width());

    ui->label->move(ui->rankButton->width()+borderSize*2,borderSize);

    if (metrics.height()*2+ui->label->height() > ui->rankButton->height()) {
        slideDistance = ui->rankButton->height() - metrics.height()*2;
        group->addAnimation(growAnimation(ui->label,QSize(0,slideDistance)));

        // resize and move the images to include image button
        QList<QPropertyAnimation*> res = arrangeImagesEditOn(QPoint(0,slideDistance));
        qDebug() << res.size();
        for (int i = 0; i < res.size(); ++i) {
            group->addAnimation(res[i]);
        }
        finalLabelHeight += metrics.height()*2;

    } else {
        group->addAnimation(growAnimation(ui->label,QSize(0,ui->rankButton->height() - ui->label->height())));
        finalLabelHeight += ui->rankButton->height() - ui->label->height();

        QList<QPropertyAnimation*> res = arrangeImagesEditOn(QPoint(0,slideDistance));
        qDebug() << res.size();
        for (int i = 0; i < res.size(); ++i) {
            group->addAnimation(res[i]);
        }
    }
    int hMax = getImagesMaxHeigth();
    saveDeltaSizeimage = hMax - int(float(hMax)*ratio);
    int slide = slideDistance - (state==states::opened?saveDeltaSizeimage:saveDeltaSizeimage - hMax - borderSize);

    group->addAnimation(growAnimation(this,QSize(0,slide)));
    group->addAnimation(slideAnimation(ui->showButton,QPoint(0,slide)));

    for (int i = 0; i < imageSlots.size(); ++i) {
        imageSlots[i]->show();
        if (state == states::retracted)
            group->addAnimation(fadeAnimation(imageSlots[i],true));
    }

    // Other buttons handeling
    interButton = (finalLabelHeight - ui->saveButton->height() - ui->cancelButton->height())/3;
    ui->saveButton->move(QPoint(-ui->saveButton->width(),borderSize+interButton));
    ui->cancelButton->move(QPoint(-ui->cancelButton->width(),borderSize+2*interButton + ui->saveButton->height()));
    interButton = (finalLabelHeight - ui->upButton->height() - ui->downButton->height())/3;
    ui->upButton->move(QPoint(this->width(),borderSize+interButton));
    ui->downButton->move(QPoint(this->width(),borderSize+2*interButton+ui->upButton->height()));

    group->addAnimation(slideAnimation(ui->rankButton,QPoint(ui->rankButton->width()+borderSize,0)));
    group->addAnimation(slideAnimation(ui->saveButton,QPoint(ui->saveButton->width()+borderSize,0)));
    group->addAnimation(slideAnimation(ui->cancelButton,QPoint(ui->cancelButton->width()+borderSize,0)));
    group->addAnimation(slideAnimation(ui->upButton,QPoint(-(ui->upButton->width()+borderMenuButton + (ui->menuButton->width()-ui->upButton->width())/2),0)));
    group->addAnimation(slideAnimation(ui->downButton,QPoint(-(ui->upButton->width()+borderMenuButton + (ui->menuButton->width()-ui->downButton->width())/2),0)));
    group->addAnimation(fadeAnimation(ui->menuButton,false));

    QObject::connect(group,&QParallelAnimationGroup::finished,[=] () {endTransition(states::opened);});
    state = states::transition;
    group->start(QAbstractAnimation::DeleteWhenStopped);

    QObject::connect(ui->saveButton,&QPushButton::released,this,&c_stepView::editSaved);
    QObject::connect(ui->cancelButton,&QPushButton::released,this,&c_stepView::editCanceled);
}

void c_stepView::editStepAnimationOff() {
    lockSize(false);

    ui->menuButton->setDisabled(false);

    QParallelAnimationGroup *group = new QParallelAnimationGroup;
    QRect rect;

    group->addAnimation(slideAnimation(ui->rankButton,QPoint(-(ui->rankButton->width()+borderSize),0)));
    group->addAnimation(slideAnimation(ui->saveButton,QPoint(-(ui->saveButton->width()+borderSize),0)));
    group->addAnimation(slideAnimation(ui->cancelButton,QPoint(-(ui->cancelButton->width()+borderSize),0)));
    group->addAnimation(fadeAnimation(ui->menuButton,true));

    QFontMetrics metrics(ui->label->document()->firstBlock().charFormat().font());
    rect = ui->label->rect();
    int newHeightLabel = getHeightText();
    int slide = 0;
    slide = ui->label->height()-newHeightLabel-8;
    if (newHeightLabel > ui->rankButton->height()) {
        QList<QPropertyAnimation*> animImages = arrangeImagesEditOff(QPoint(0,-slide));
        for (int i = 0; i < animImages.size(); ++i) {
            group->addAnimation(animImages[i]);
        }
        int resize = slide + (imageSlots.isEmpty()?0:(ui->showButton->pos().y() - imageSlots[0]->pos().y() - getImagesMaxHeigth() - borderSize )) - saveDeltaSizeimage;
        group->addAnimation(growAnimation(this,QSize(0,-resize)));
        group->addAnimation(slideAnimation(ui->showButton,QPoint(0,-resize)));
    }
    group->addAnimation(growAnimation(ui->label,QSize(0,-slide)));

    group->addAnimation(slideAnimation(ui->upButton,QPoint(ui->upButton->width()+borderMenuButton + (ui->menuButton->width()-ui->upButton->width())/2,0)));
    group->addAnimation(slideAnimation(ui->downButton,QPoint(ui->downButton->width()+borderMenuButton + (ui->menuButton->width()-ui->downButton->width())/2,0)));

    int nextState = state;
    qDebug() << state;
    QObject::connect(group,&QParallelAnimationGroup::finished,[=] () {endStepAnimationOff(nextState);});
    state = states::transition;
    group->start(QAbstractAnimation::DeleteWhenStopped);
}

void c_stepView::endStepAnimationOff(int _state) {
    ui->label->setReadOnly(true);
    ui->label->setStyleSheet("QTextEdit {"
                             "  border : 1px solid white;"
                             "  background: transparent;"
                             "}");
    ui->label->raise();
    ui->label->setFixedSize(QSize(QWIDGETSIZE_MAX,QWIDGETSIZE_MAX));
    ui->label->setFixedWidth(this->width() - ui->rankButton->width() - 2*borderSize - ui->menuButton->width()-2*borderMenuButton );

    ui->label->move(ui->rankButton->width()+borderSize*2,borderSize);
    this->endTransition(_state);
}

void c_stepView::editSaved() {

    imageList.append(newImageList);
    imageSlots.append(newImageSlots);

    for (int i = 0; i < newImageSlots.size(); ++i) {
        addImageButtons[0]->deleteLater();
        addImageButtons.removeAt(0);
    }

    newImageList.clear();
    newImageSlots.clear();


    editStepAnimationOff();
    QObject::disconnect(ui->saveButton,&QPushButton::released,this,&c_stepView::editSaved);
    QObject::disconnect(ui->cancelButton,&QPushButton::released,this,&c_stepView::editCanceled);

    QString res;
    for (int i = 0; i < ui->label->document()->blockCount(); ++i) {
        res.append(ui->label->document()->findBlockByNumber(i).text());
        res.append("\n");
    }
    res.remove(QRegExp("\n$"));
    step->setDescription(res);
    emit saved(step);
}

void c_stepView::editCanceled() {
    ui->label->clear();
    ui->label->append(step->getDescription());
    ui->label->setAlignment(Qt::AlignJustify);


    for (int i = 0; i < newImageSlots.size(); ++i) {
        newImageSlots[i]->deleteLater();
    }
    newImageSlots.clear();
    newImageList.clear();

    editStepAnimationOff();
    QObject::disconnect(ui->saveButton,&QPushButton::released,this,&c_stepView::editSaved);
    QObject::disconnect(ui->cancelButton,&QPushButton::released,this,&c_stepView::editCanceled);
}

void c_stepView::upEdit() {
    if (rankEdit > 0) {
        rankEdit--;
        emit new_rank(rankEdit);
    }
}

void c_stepView::downEdit() {
    rankEdit++;
    emit new_rank(rankEdit);
}

void c_stepView::editAreaSizeChanged(int increment) {
    lockSize(false);

    QParallelAnimationGroup *group = new QParallelAnimationGroup;
    QRect rect;

    group->addAnimation(growAnimation(ui->label,QSize(0,increment)));
    group->addAnimation(growAnimation(this,QSize(0,increment)));
    group->addAnimation(slideAnimation(ui->showButton,QPoint(0,increment)));

    int nextState = state;
    QObject::connect(group,&QParallelAnimationGroup::finished,[=] () {endTransition(nextState);});
    state = states::transition;
    group->start(QAbstractAnimation::DeleteWhenStopped);
}

void c_stepView::endTransition(int _state) {
    qDebug() << "[" << QTime::currentTime() << "]" << "endTransition, _state :" << _state;
    switch (_state) {
    case states::opened :
        state = states::opened;
        this->setFixedSize(this->size());
        break;
    case states::retracted :
        state = states::retracted;
        this->setFixedSize(this->size());
        for (int i = 0; i < imageSlots.size(); ++i) {
            imageSlots[i]->hide();
        }
        break;
    default:
        return ;
    }
    qDebug() << state;
}

void c_stepView::handleAddimage() {
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),"/home",tr("Images (*.png *.xpm *.jpg)"),nullptr,QFileDialog::DontUseNativeDialog);
    if (imageSlots.size() < maxNumberImages) {
        QPixmap pix = QPixmap(fileName);
        if (!pix.isNull()) {
            QParallelAnimationGroup *group = new QParallelAnimationGroup;

            int maxW = (this->size().width() - (imageSlots.size()-1)*interImageSpace - 2*borderSize)/imageSlots.size();

            maxW = maxW>maxSizeImage.width()?maxSizeImage.width():maxW;
            QSize labelsize(maxW>maxSizeImage.width()?maxSizeImage.width():maxW,maxSizeImage.height());

            newImageList.push_back(pix);
            newImageSlots.push_back(new QLabel(this));

            pix = pix.scaled(labelsize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

            if (int(float(labelsize.height())*ratio)>getImagesMaxHeigth()) {
                lockSize(false);
                group->addAnimation(slideAnimation(ui->showButton,QPoint(0,int(float(labelsize.height())*ratio)-getImagesMaxHeigth())));
                group->addAnimation(growAnimation(this,QSize(0,int(float(labelsize.height())*ratio)-getImagesMaxHeigth())));
            }

            newImageSlots.last()->setPixmap(pix);
            newImageSlots.last()->setBackgroundRole(QPalette::Base);
            newImageSlots.last()->setScaledContents(true);
            newImageSlots.last()->hide();

            newImageSlots.last()->move(addImageButtons[newImageSlots.size()-1]->pos());
            addImageButtons[newImageSlots.size()-1]->hide();
            newImageSlots.last()->show();

            QPropertyAnimation *animation = new QPropertyAnimation( newImageSlots.last(), "geometry");
            animation->setDuration(1000);
            QRect rect = newImageSlots.last()->rect();
            rect.setTopLeft(newImageSlots.last()->pos());
            rect.setSize(QSize(0,0));
            animation->setStartValue(rect);
            rect.setSize(QSize(int(float(pix.width())*ratio),int(float(pix.height())*ratio)));
            animation->setEndValue(rect);
            animation->setEasingCurve(QEasingCurve::InOutQuart);
            group->addAnimation(animation);

            QObject::connect(group,&QParallelAnimationGroup::finished,[=] () {endTransition(states::opened);});
            state = states::transition;

            group->start(QAbstractAnimation::DeleteWhenStopped);
        }
    }
}

int c_stepView::getHeightText() {
    QRect rect = ui->label->rect();
    int res = 0;
    for (int i = 0; i < ui->label->document()->blockCount(); ++i) {
        QFontMetrics metrics(ui->label->document()->findBlockByNumber(i).charFormat().font());
        res += metrics.boundingRect(rect,Qt::TextWordWrap,ui->label->document()->findBlockByNumber(i).text()).size().height();
    }
    return res;
}

void c_stepView::lockSize(bool flag) {
    if (flag) {
        this->setFixedSize(this->size());
    } else {
        this->setFixedSize(QSize(QWIDGETSIZE_MAX,QWIDGETSIZE_MAX));
        this->setFixedWidth(this->size().width());
    }
}

QPropertyAnimation *c_stepView::slideAnimation(QWidget *parent, QPoint slide, QSize growth) {
    QRect rect;
    QSize size;
    QPropertyAnimation *animation = new QPropertyAnimation(parent,"geometry");
    animation->setDuration(1000);
    rect = parent->rect();
    size = parent->size();
    rect.setTopLeft(parent->pos());
    rect.setSize(size);
    animation->setStartValue(rect);
    rect.setTopLeft(parent->pos() + slide);
    rect.setSize(size + growth);
    animation->setEndValue(rect);
    animation->setEasingCurve(QEasingCurve::InOutQuart);

    return animation;
}

QPropertyAnimation *c_stepView::growAnimation(QWidget *parent, QSize growth) {
    QPropertyAnimation *animation = new QPropertyAnimation(parent, "size");
    animation->setDuration(1000);
    animation->setStartValue(parent->size());
    animation->setEndValue(parent->size() + growth);

    animation->setEasingCurve(QEasingCurve::InOutQuart);

    return animation;
}

QPropertyAnimation *c_stepView::fadeAnimation(QWidget *parent, bool up) {
    QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(parent);
    parent->setGraphicsEffect(effect);
    QPropertyAnimation *animation = new QPropertyAnimation(effect, "opacity");
    animation->setDuration(1000);
    animation->setStartValue(up ? 0.0 : 1.0);
    animation->setEndValue(up ? 1.0 : 0.0);
    animation->setEasingCurve(QEasingCurve::InOutQuart);

    return animation;
}

QList<QPropertyAnimation *> c_stepView::arrangeImagesEditOn(QPoint verticalShift) {
    QList<QPropertyAnimation*> res;
    QList<QPoint> newPos = arrangeImages(modes::edition,verticalShift);
    QPoint shift;
    QRect rect;
    for (int i = 0; i < imageSlots.size(); ++i) {
        imageSlots[i]->setFixedSize(QSize(QWIDGETSIZE_MAX,QWIDGETSIZE_MAX));
        QSize growth(-int((1.0f-ratio)*float(imageSlots[i]->width())),-int((1.0f-ratio)*float(imageSlots[i]->height())));
        shift = newPos[i] - imageSlots[i]->pos();
        res.push_back(slideAnimation(imageSlots[i],shift,growth));
    }
    for (int i = 0; i < addImageButtons.size(); ++i) {
        addImageButtons[i]->deleteLater();
    }
    addImageButtons.clear();
    for (int i = imageSlots.size(); i < maxNumberImages; ++i) {
        addImageButtons.push_back(new QPushButton("Add Image",this));
        addImageButtons.last()->move(newPos[i]);
        addImageButtons.last()->show();
        addImageButtons.last()->lower();

        QObject::connect(addImageButtons.last(),&QPushButton::released,this,&c_stepView::handleAddimage);

        QPropertyAnimation *animation = new QPropertyAnimation( addImageButtons.last(), "geometry");
        animation->setDuration(1000);
        rect = addImageButtons.last()->rect();
        rect.setTopLeft(addImageButtons.last()->pos());
        rect.setSize(QSize(0,0));
        animation->setStartValue(rect);
        rect.setSize(QSize(int(float(maxSizeImage.width())*ratio),int(float(getImagesMaxHeigth())*ratio)));
        animation->setEndValue(rect);
        animation->setEasingCurve(QEasingCurve::InOutQuart);
        res.push_back(animation);
    }
    return res;
}

QList<QPropertyAnimation *> c_stepView::arrangeImagesEditOff(QPoint verticalShift) {
    QList<QPropertyAnimation*> res;
    QList<QPoint> newPos = arrangeImages(modes::display,verticalShift);
    QPoint shift;
    QRect rect;
    for (int i = 0; i < imageSlots.size(); ++i) {
        imageSlots[i]->setFixedSize(QSize(QWIDGETSIZE_MAX,QWIDGETSIZE_MAX));
        QSize growth(-int((1.0f-ratio)*float(imageSlots[i]->width())),-int((1.0f-ratio)*float(imageSlots[i]->height())));
        shift = newPos[i] - imageSlots[i]->pos();
        res.push_back(slideAnimation(imageSlots[i],shift,growth));
        imageSlots[i]->raise();
    }
    for (int i = 0; i < addImageButtons.size(); ++i) {
        QPropertyAnimation *animation = new QPropertyAnimation( addImageButtons[i], "geometry");
        animation->setDuration(1000);
        rect = addImageButtons[i]->rect();
        rect.setTopLeft(addImageButtons[i]->pos());
        rect.setSize(addImageButtons[i]->size());
        animation->setStartValue(rect);
        rect.setSize(QSize(0,0));
        animation->setEndValue(rect);
        animation->setEasingCurve(QEasingCurve::InOutQuart);
        res.push_back(animation);
        QObject::disconnect(addImageButtons[i],&QPushButton::released,this,&c_stepView::handleAddimage);
    }
    return res;
}

QList<QPoint> c_stepView::arrangeImages(bool mode, QPoint verticalShift) {
    QList<QPoint> res;
    int totalWidth = 0;
    for (int i = 0; i < imageSlots.size(); ++i) {
        totalWidth += imageSlots[i]->width();
    }
    if (mode == modes::display) {
        int widthTotalAvailableAfter = this->width() - 2*borderSize - (imageSlots.size()-1)*interImageSpace;
        widthTotalAvailableAfter = (widthTotalAvailableAfter > (maxSizeImage.width()*imageSlots.size()) ? maxSizeImage.width()*(imageSlots.size()) : widthTotalAvailableAfter);
        ratio = float(widthTotalAvailableAfter)/float(totalWidth);

        QPoint point = QPoint((this->size().width() - 2*borderSize - widthTotalAvailableAfter - ((imageSlots.size()-1)*interImageSpace))/2 + borderSize ,borderSize*2 + std::max(ui->rankButton->height(),ui->label->height())) + verticalShift;
        for (int i = 0; i < imageSlots.size(); ++i) {
            res.push_back(point);
            point += QPoint(int(float(imageSlots[i]->size().width())*ratio) + interImageSpace,0);
        }
    } else {
        int widthTotalAvailableAfter = this->width() - 2*borderSize - (maxNumberImages-1)*interImageSpace;
        widthTotalAvailableAfter = (widthTotalAvailableAfter > (maxSizeImage.width()*maxNumberImages) ? maxSizeImage.width()*(maxNumberImages) : widthTotalAvailableAfter);
        ratio = float(widthTotalAvailableAfter)/float(totalWidth + ( maxSizeImage.width()* (maxNumberImages-imageSlots.size())));

        QPoint point = QPoint(borderSize ,borderSize*2 + std::max(ui->rankButton->height(),ui->label->height())) + verticalShift;
        for (int i = 0; i < imageSlots.size(); ++i) {
            res.push_back(point);
            point += QPoint(int(float(imageSlots[i]->size().width())*ratio) + interImageSpace,0);
        }

        for (int i = imageSlots.size(); i < maxNumberImages; ++i) {
            res.push_back(point);
            point += QPoint(int(float(maxSizeImage.width())*ratio) + interImageSpace,0);
        }
    }
    return res;
}

int c_stepView::getImagesMaxHeigth() {
    int max = 0;
    for (int i = 0; i < imageSlots.size(); ++i) {
        if (imageSlots[i]->height() > max)
            max = imageSlots[i]->height();
    }
    for (int i = 0; i < newImageSlots.size(); ++i) {
        if (newImageSlots[i]->height() > max)
            max = newImageSlots[i]->height();
    }
    return max;
}
