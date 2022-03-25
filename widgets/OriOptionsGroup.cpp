#include "OriOptionsGroup.h"

#include <QDebug>
#include <QCheckBox>
#include <QRadioButton>
#include <QVBoxLayout>

namespace Ori {
namespace Widgets {

//------------------------------------------------------------------------------
//                                 OptionsGroup
//------------------------------------------------------------------------------

OptionsGroup::OptionsGroup(bool radio, QWidget *parent)
    : QGroupBox(parent), _radio(radio)
{
    setLayout(new QVBoxLayout);
}

OptionsGroup::OptionsGroup(const QString &title, bool radio, QWidget *parent)
    : QGroupBox(title, parent), _radio(radio)
{
    setLayout(new QVBoxLayout);
}

#ifdef Q_COMPILER_INITIALIZER_LISTS
OptionsGroup::OptionsGroup(const QString &title, std::initializer_list<QString> options,
    bool radio, QWidget *parent) : OptionsGroup(title, radio, parent)
{
    addOptions(options);
}
#endif

void OptionsGroup::addOption(const QString &title)
{
    addOption(_options.size(), title);
}

void OptionsGroup::addOption(int id, const QString &title)
{
    if (_options.contains(id))
    {
        qCritical() << "There is already an option with id" << id;
        return;
    }
    QAbstractButton *check;
    if (_radio)
        check = new QRadioButton(title);
    else
        check = new QCheckBox(title);
    connect(check, SIGNAL(clicked(bool)), this, SLOT(buttonClicked(bool)));
    _options.insert(id, check);
    layout()->addWidget(check);
}

#ifdef Q_COMPILER_INITIALIZER_LISTS
void OptionsGroup::addOptions(std::initializer_list<QString> options)
{
    for (const QString& option : options) addOption(option);
}
#endif

bool OptionsGroup::option(int id) const
{
    QAbstractButton* o = getOption(id);
    if (!o) return false;
    if (_radio)
        return o->isChecked();
    else
        return qobject_cast<QCheckBox*>(o)->checkState() == Qt::Checked;
}

void OptionsGroup::setOption(int id, bool value)
{
    QAbstractButton* o = getOption(id);
    if (!o) return;
    if (_radio)
        o->setChecked(value);
    else
        qobject_cast<QCheckBox*>(o)->setCheckState(value? Qt::Checked: Qt::Unchecked);
}

int OptionsGroup::option()
{
    if (!_radio)
    {
        qCritical() << "Method is valid for radio group only";
        return -1;
    }
    QMapIterator<int, QAbstractButton*> o(_options);
    while (o.hasNext())
    {
        o.next();
        if (o.value()->isChecked())
            return o.key();
    }
    qWarning() << "There is no checked option in radio group";
    return -1;
}

void OptionsGroup::buttonClicked(bool checked)
{
    if (!checked) return;
    QObject *option = sender();
    QMapIterator<int, QAbstractButton*> o(_options);
    while (o.hasNext())
    {
        o.next();
        if (o.value() == option && o.value()->isChecked())
        {
            emit optionChecked(o.key());
            return;
        }
    }
}

QAbstractButton* OptionsGroup::getOption(int id) const
{
    if (!_options.contains(id))
    {
        qCritical() << "There is no option with id" << id;
        return nullptr;
    }
    return _options[id];
}

void OptionsGroup::addControls(std::initializer_list<QObject*> controls)
{
    for (auto control : controls)
    {
        auto layout = qobject_cast<QLayout*>(control);
        if (layout)
            qobject_cast<QVBoxLayout*>(this->layout())->addLayout(layout);
        auto widget = qobject_cast<QWidget*>(control);
        if (widget)
            this->layout()->addWidget(widget);
    }
}

//------------------------------------------------------------------------------
//                               OptionsGroupV2
//------------------------------------------------------------------------------

OptionsGroupV2::OptionsGroupV2(bool radio, QWidget *parent)
    : QGroupBox(parent), _radio(radio)
{
    setLayout(new QVBoxLayout);
}

OptionsGroupV2::OptionsGroupV2(const QString &title, bool radio, QWidget *parent)
    : QGroupBox(title, parent), _radio(radio)
{
    setLayout(new QVBoxLayout);
}

OptionsGroupV2::OptionsGroupV2(const QString &title, const QVector<QPair<QString, QString>>& options, bool radio, QWidget *parent)
    : OptionsGroupV2(title, radio, parent)
{
    addOptions(options);
}

void OptionsGroupV2::addOption(const QString& id, const QString &title)
{
    if (_options.contains(id))
    {
        qCritical() << "There is already an option with id" << id;
        return;
    }
    QAbstractButton *check;
    if (_radio)
        check = new QRadioButton(title);
    else
        check = new QCheckBox(title);
    connect(check, SIGNAL(clicked(bool)), this, SLOT(buttonClicked(bool)));
    _options.insert(id, check);
    layout()->addWidget(check);
}

void OptionsGroupV2::addOptions(const QVector<QPair<QString, QString>>& options)
{
    foreach (const auto& it, options)
        addOption(it.first, it.second);
}

bool OptionsGroupV2::option(const QString& id) const
{
    QAbstractButton* o = getButton(id);
    if (!o) return false;
    if (_radio)
        return o->isChecked();
    else
        return qobject_cast<QCheckBox*>(o)->checkState() == Qt::Checked;
}

void OptionsGroupV2::setOption(const QString& id, bool value)
{
    QAbstractButton* o = getButton(id);
    if (!o) return;
    if (_radio)
        o->setChecked(value);
    else
        qobject_cast<QCheckBox*>(o)->setCheckState(value? Qt::Checked: Qt::Unchecked);
}

QString OptionsGroupV2::option() const
{
    if (!_radio)
    {
        qCritical() << "Method is valid for radio group only";
        return QString();
    }
    auto it = _options.constBegin();
    while (it != _options.constEnd())
    {
        if (it.value()->isChecked())
            return it.key();
        it++;
    }
    qWarning() << "There is no checked option in radio group";
    return QString();
}

void OptionsGroupV2::buttonClicked(bool checked)
{
    if (!checked) return;
    QObject *option = sender();
    auto it = _options.constBegin();
    while (it != _options.constEnd())
    {
        if (it.value() == option && it.value()->isChecked())
        {
            emit optionChecked(it.key());
            return;
        }
        it++;
    }
}

QAbstractButton* OptionsGroupV2::getButton(const QString& id) const
{
    if (!_options.contains(id))
    {
        qCritical() << "There is no option with id" << id;
        return nullptr;
    }
    return _options[id];
}

void OptionsGroupV2::addControls(std::initializer_list<QObject*> controls)
{
    for (auto control : controls)
    {
        auto layout = qobject_cast<QLayout*>(control);
        if (layout)
            qobject_cast<QVBoxLayout*>(this->layout())->addLayout(layout);
        auto widget = qobject_cast<QWidget*>(control);
        if (widget)
            this->layout()->addWidget(widget);
    }
}

} // namespace Widgets
} // namespace Gui
