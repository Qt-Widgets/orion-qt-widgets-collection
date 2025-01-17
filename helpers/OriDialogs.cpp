#include "OriDialogs.h"

#include <QApplication>
#include <QAbstractButton>
#include <QBoxLayout>
#include <QDebug>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QGlobalStatic>
#include <QIcon>
#include <QLabel>
#include <QStyle>
#include <QMessageBox>

namespace Ori {
namespace Dlg {

namespace Mock {

static bool isActive = false;
static DialogKind lastDialog = DialogKind::none;
static int nextResult = QMessageBox::StandardButton::NoButton;

void setActive(bool on) { isActive = on; }
void resetLastDialog() { lastDialog = DialogKind::none; }
DialogKind getLastDialog() { return lastDialog; }
void setNextResult(int res) { nextResult = res; }

} // namespace Mock

void info(const QString& msg)
{
    if (Mock::isActive)
    {
        Mock::lastDialog = Mock::DialogKind::info;
        return;
    }

    QMessageBox::information(qApp->activeWindow(), qApp->applicationName(), msg, QMessageBox::Ok, QMessageBox::Ok);
}

void warning(const QString& msg)
{
    if (Mock::isActive)
    {
        Mock::lastDialog = Mock::DialogKind::warning;
        return;
    }

    QMessageBox::warning(qApp->activeWindow(), qApp->applicationName(), msg, QMessageBox::Ok, QMessageBox::Ok);
}

void error(const QString& msg)
{
    if (Mock::isActive)
    {
        Mock::lastDialog = Mock::DialogKind::error;
        return;
    }

    QMessageBox::critical(qApp->activeWindow(), qApp->applicationName(), msg, QMessageBox::Ok, QMessageBox::Ok);
}

bool yes(const QString& msg)
{
    if (Mock::isActive)
    {
        Mock::lastDialog = Mock::DialogKind::yes;
        return Mock::nextResult == QMessageBox::Yes;
    }

    return (QMessageBox::question(qApp->activeWindow(), qApp->applicationName(), msg,
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes);
}

bool ok(const QString& msg)
{
    if (Mock::isActive)
    {
        Mock::lastDialog = Mock::DialogKind::ok;
        return Mock::nextResult == QMessageBox::Ok;
    }

    return (QMessageBox::question(qApp->activeWindow(), qApp->applicationName(), msg,
        QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel) == QMessageBox::Ok);
}

int yesNoCancel(const QString& msg)
{
    if (Mock::isActive)
    {
        Mock::lastDialog = Mock::DialogKind::yesNoCancel;
        return Mock::nextResult;
    }

    return QMessageBox::question(qApp->activeWindow(), qApp->applicationName(), msg,
        QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Yes);
}

int yesNoCancel(QString& msg)
{
    if (Mock::isActive)
    {
        Mock::lastDialog = Mock::DialogKind::yesNoCancel;
        return Mock::nextResult;
    }

    return QMessageBox::question(qApp->activeWindow(), qApp->applicationName(), msg,
        QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Yes);
}

//------------------------------------------------------------------------------
//                         Ori::Dlg::InputTextEditor
//------------------------------------------------------------------------------

InputTextEditor::InputTextEditor(QWidget* parent) : QLineEdit(parent)
{
}

QSize InputTextEditor::sizeHint() const
{
    auto s = QLineEdit::sizeHint();
    s.setWidth(s.width() * _widthFactor);
    return s;
}

//------------------------------------------------------------------------------
//                            Ori::Dlg::inputText
//------------------------------------------------------------------------------

QString inputText(const QString& label, const QString& value)
{
    bool ok;
    QString text = inputText(label, value, &ok);
    return ok? text: QString();
}

QString inputText(const QString& label, const QString& value, bool *ok)
{
    auto newValue = value;

    auto editor = new InputTextEditor();
    editor->setText(value);
    auto s = editor->size();
    editor->resize(s.width() * 2, s.height());

    QWidget content;
    auto layout = new QVBoxLayout(&content);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(new QLabel(label));
    layout->addWidget(editor);

    *ok = Dialog(&content, false)
            .withContentToButtonsSpacingFactor(2)
            .exec();

    if (*ok)
        newValue = editor->text();

    return newValue;
}

//------------------------------------------------------------------------------

QString getSaveFileName(const QString& title, const QString& filter, const QString& defaultExt)
{
    QString fileName = QFileDialog::getSaveFileName(qApp->activeWindow(), title, QString(), filter);
    if (fileName.isEmpty()) return QString();

    if (QFileInfo(fileName).suffix().isEmpty())
    {
        if (fileName.endsWith('.'))
            return fileName + defaultExt;
        return fileName + '.' + defaultExt;
    }

    return fileName;
}

//------------------------------------------------------------------------------
//                        Ori::Dlg::showDialogWithPrompt
//------------------------------------------------------------------------------

bool showDialogWithPrompt(Qt::Orientation orientation, const QString& prompt, QWidget *widget, const QString& title, const QString &icon)
{
    auto oldParent = qobject_cast<QWidget*>(widget->parent());

    QWidget content;
    QBoxLayout *layout;
    if (orientation == Qt::Vertical)
        layout = new QVBoxLayout(&content);
    else
        layout = new QHBoxLayout(&content);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(new QLabel(prompt));
    layout->addWidget(widget);
    bool ok = Dialog(&content, false)
            .withTitle(title)
            .withIconPath(icon)
            .withContentToButtonsSpacingFactor(2)
            .exec();

    // Restore parent to prevent the layout from deletion the widget
    widget->setParent(oldParent);

    return ok;
}

bool showDialogWithPromptH(const QString& prompt, QWidget *widget, const QString& title, const QString &icon)
{
    return showDialogWithPrompt(Qt::Horizontal, prompt, widget, title, icon);
}

bool showDialogWithPromptV(const QString& prompt, QWidget *widget, const QString& title, const QString &icon)
{
    return showDialogWithPrompt(Qt::Vertical, prompt, widget, title, icon);
}

//------------------------------------------------------------------------------

bool show(QDialog* dlg)
{
    return dlg->exec() == QDialog::Accepted;
}

void setDlgTitle(QWidget *dlg, const QString& title)
{
    dlg->setWindowTitle(title.isEmpty()? qApp->applicationName(): title);
}

/**
    Assign dialog window icon.

    On MacOS the icon of active dialog overrides application icon on the dock.
    It is not what we want. As no icon is displayed in window titlebar so we have nothing to do.

    Ubuntu Unity does not display icons in window titlebar,
    but there are another desktops that can show icons (xfce, KDE).
*/
void setDlgIcon(QWidget *dlg, const QString &path)
{
#ifdef Q_OS_MACOS
    Q_UNUSED(dlg)
    Q_UNUSED(path)
#else
    if (path.isEmpty()) return;
    QIcon icon(path);
    if (icon.isNull()) return;
    dlg->setWindowIcon(icon);
#endif
}

//------------------------------------------------------------------------------
//                             Ori::Dlg::Dialog
//------------------------------------------------------------------------------

namespace {
using SavedSizesMap = QMap<QString, QSize>;
Q_GLOBAL_STATIC(SavedSizesMap, __savedSizes)
}

Dialog::Dialog(QWidget* content, bool ownContent): _content(content), _ownContent(ownContent)
{
    _backupContentParent = _content->parentWidget();
}

Dialog::~Dialog()
{
    if (_dialog) delete _dialog;
}

bool Dialog::exec()
{
    if (!_dialog) makeDialog();
    if (_activeWidget) _activeWidget->setFocus();
    bool res = _dialog->exec() == QDialog::Accepted;
    if (!_ownContent)
    {
        // Restoring ownership prevents widget deletion together with layout
        _contentLayout->removeWidget(_content);
        _content->setParent(_backupContentParent);
    }
    if (!_persistenceId.isEmpty())
        (*__savedSizes)[_persistenceId] = _dialog->size();
    return res;
}

void Dialog::makeDialog()
{
    // Dialog window
    _dialog = new QDialog(qApp->activeWindow());

    auto flags = _dialog->windowFlags();
    flags.setFlag(Qt::WindowContextHelpButtonHint, false);
    _dialog->setWindowFlags(flags);

    setDlgTitle(_dialog, _title);
    setDlgIcon(_dialog, _iconPath);
    if (!_initialSize.isEmpty())
        _dialog->resize(_initialSize);
    else if (!_persistenceId.isEmpty() && __savedSizes->contains(_persistenceId))
        _dialog->resize((*__savedSizes)[_persistenceId]);
    QVBoxLayout* dialogLayout = new QVBoxLayout(_dialog);

    // Dialog content
    if (!_prompt.isEmpty())
    {
        if (_isPromptVertical)
            _contentLayout = new QVBoxLayout;
        else
            _contentLayout = new QHBoxLayout;
        _contentLayout->setContentsMargins(0, 0, 0, 0);
        _contentLayout->addWidget(new QLabel(_prompt));
        dialogLayout->addLayout(_contentLayout);
    }
    else
    {
        _contentLayout = dialogLayout;
        if (_skipContentMargins)
            _contentLayout->setContentsMargins(0, 0, 0, 0);
    }
    _contentLayout->addWidget(_content);

    auto style = qApp->style();

    // Content-to-buttons space
    if (_fixedContentSize)
        dialogLayout->addStretch();
    if (_contentToButtonsSpacingFactor > 1)
    {
        int defaultSpacing = style->pixelMetric(QStyle::PM_LayoutVerticalSpacing);
        dialogLayout->addSpacing(defaultSpacing * (_contentToButtonsSpacingFactor - 1));
    }

    // Dialog buttons
    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel |
        (_onHelpRequested ? QDialogButtonBox::Help : QDialogButtonBox::NoButton) |
        (_applyHandler ? QDialogButtonBox::Apply : QDialogButtonBox::NoButton)
    );
    qApp->connect(buttonBox, &QDialogButtonBox::accepted, _dialog, [this]{ acceptDialog(); });
    qApp->connect(buttonBox, &QDialogButtonBox::rejected, _dialog, &QDialog::reject);
    if (_connectOkToContentApply)
        qApp->connect(_dialog, SIGNAL(accepted()), _content, SLOT(apply()));
    foreach (const auto& signal, _acceptSignals)
        qApp->connect(signal.first ? signal.first : _content, signal.second, _dialog, SLOT(accept()));
    if (_applyHandler)
        qApp->connect(buttonBox, &QDialogButtonBox::clicked, [buttonBox, this](QAbstractButton *button){
            if ((void*)button == (void*)buttonBox->button(QDialogButtonBox::Apply))
                _applyHandler();
        });
    if (_onHelpRequested)
        qApp->connect(buttonBox, &QDialogButtonBox::helpRequested, _onHelpRequested);

    // By default dialogLayout provides margins
    // When skipping content margins we still want to have margins around buttons
    if (_skipContentMargins)
        buttonBox->setContentsMargins(
            style->pixelMetric(QStyle::PM_LayoutLeftMargin),
            0,
            style->pixelMetric(QStyle::PM_LayoutRightMargin),
            style->pixelMetric(QStyle::PM_LayoutBottomMargin));

    dialogLayout->addWidget(buttonBox);

    foreach (auto button, buttonBox->buttons())
        if (buttonBox->buttonRole(button) == QDialogButtonBox::AcceptRole)
        {
            _okButton = button;
            break;
        }

    if (_onDlgReady) _onDlgReady();
}

void Dialog::acceptDialog()
{
    if (_verify)
    {
        QString res = _verify();
        if (!res.isEmpty())
        {
            warning(res);
            return;
        }
    }
    _dialog->accept();
}

QSize Dialog::size() const
{
    return _dialog ? _dialog->size() : QSize();
}

Dialog& Dialog::withAcceptSignal(const char* signal)
{
    _acceptSignals << QPair<QObject*, const char*>(nullptr, signal);
    return *this;
}

Dialog& Dialog::withAcceptSignal(QObject* sender, const char* signal)
{
    _acceptSignals << QPair<QObject*, const char*>(sender, signal);
    return *this;
}

} // namespace Dlg
} // namespace Ori
