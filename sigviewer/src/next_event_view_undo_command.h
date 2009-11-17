#ifndef NEXT_EVENT_VIEW_UNDO_COMMAND_H
#define NEXT_EVENT_VIEW_UNDO_COMMAND_H

#include "signal_browser/signal_browser_model_4.h"

#include <QUndoCommand>
#include <QSharedPointer>
#include <QPointF>

namespace BioSig_
{

class EventGraphicsItem;

class NextEventViewUndoCommand : public QUndoCommand
{
public:
    //-------------------------------------------------------------------------
    NextEventViewUndoCommand(SignalBrowserModel& signal_browser_model);

    //-------------------------------------------------------------------------
    /// destructor
    virtual ~NextEventViewUndoCommand ();

    //-------------------------------------------------------------------------
    virtual void undo ();

    //-------------------------------------------------------------------------
    virtual void redo ();


private:
    QSharedPointer<EventGraphicsItem> previously_selected_event_;
    QPointF previous_view_position_;

    SignalBrowserModel& signal_browser_model_;

    //-------------------------------------------------------------------------
    /// copy-constructor disabled
    NextEventViewUndoCommand (NextEventViewUndoCommand const &);

    //-------------------------------------------------------------------------
    /// assignment-operator disabled
    NextEventViewUndoCommand& operator= (NextEventViewUndoCommand const &);


};

}

#endif // NEXT_EVENT_VIEW_UNDO_COMMAND_H
