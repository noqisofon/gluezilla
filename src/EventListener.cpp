#include "EventListener.h"

NS_IMPL_ISUPPORTS1(EventListener, nsIDOMEventListener)

#undef NS_LITERAL_STRING

#define NS_LITERAL_STRING(_str_)  (PRUnichar *)_str_


NS_IMETHODIMP
EventListener::HandleEvent (nsIDOMEvent *domEvent)
{
	nsEmbedString type;
	domEvent->GetType (type);
	
	
	PRBool ret = PR_FALSE;
	nsCOMPtr <nsIDOMKeyEvent> keyEvent = do_QueryInterface(domEvent);
	nsCOMPtr <nsIDOMMouseEvent> mouseEvent = do_QueryInterface(domEvent);

	if (keyEvent) {
		ret = OnKey (keyEvent, type);
	} else if (mouseEvent) {
		ret = OnMouse (mouseEvent, type);	
	} else if (type.Equals(NS_LITERAL_STRING("focus"))) {
		ret = events->OnFocus ();
	} else if (type.Equals(NS_LITERAL_STRING("blur"))) {
		ret = events->OnBlur ();
	} else if (type.Equals(NS_LITERAL_STRING("load"))) {
		events->OnLoad ();
	} else if (type.Equals(NS_LITERAL_STRING("unload"))) {
		events->OnUnload ();
	} else {
		
//		PRUnichar * t = (PRUnichar*)NS_StringCloneData(type);
		nsEmbedCString s = NS_ConvertUTF16toUTF8 (type);
		PRINT2 ("Calling Generic with %s\n", s.get());
		events->OnGeneric (type.get());
	}

	if (ret) {
		domEvent->StopPropagation();
		domEvent->PreventDefault();
	}

	return ret;
}

nsresult 
EventListener::OnKey (nsCOMPtr <nsIDOMKeyEvent> keyEvent, nsEmbedString type)
{
	nsresult ret = PR_FALSE;
	ModifierKeys mkey;
	keyEvent->GetAltKey (&(mkey.altKey));
	keyEvent->GetCtrlKey (&(mkey.ctrlKey));
	keyEvent->GetMetaKey (&(mkey.metaKey));
	keyEvent->GetShiftKey (&(mkey.shiftKey));

	KeyInfo keyInfo;
	keyEvent->GetCharCode (&(keyInfo.charCode));
	keyEvent->GetKeyCode (&(keyInfo.keyCode));

	nsCOMPtr <nsIDOMEventTarget> target;
	keyEvent->GetTarget(getter_AddRefs (target));
	nsCOMPtr <nsIDOMNode> node = do_QueryInterface(target);
	
	
	// Return TRUE from your signal handler to mark the event as consumed.
	if (type.Equals(NS_LITERAL_STRING("keyup")))
		ret = events->OnDomKeyUp (keyInfo, mkey, node);
	else if (type.Equals(NS_LITERAL_STRING("keydown")))
		ret = events->OnDomKeyDown (keyInfo, mkey, node);
	else if (type.Equals(NS_LITERAL_STRING("keypress")))
		ret = events->OnDomKeyPress (keyInfo, mkey, node);
	return PR_FALSE;
}

nsresult 
EventListener::OnMouse (nsCOMPtr <nsIDOMMouseEvent> mouseEvent, nsEmbedString type)
{
	nsresult ret = PR_FALSE;
	ModifierKeys modifiers;
	MouseInfo mouseInfo;
	mouseEvent->GetAltKey (&(modifiers.altKey));
	mouseEvent->GetCtrlKey (&(modifiers.ctrlKey));
	mouseEvent->GetMetaKey (&(modifiers.metaKey));
	mouseEvent->GetShiftKey (&(modifiers.shiftKey));

	
	mouseEvent->GetClientX (&(mouseInfo.clientX));
	mouseEvent->GetClientY (&(mouseInfo.clientY));
	mouseEvent->GetScreenX (&(mouseInfo.screenX));
	mouseEvent->GetScreenY (&(mouseInfo.screenY));


	mouseEvent->GetButton (&(mouseInfo.button));
	
	nsCOMPtr <nsIDOMEventTarget> target;
	mouseEvent->GetTarget(getter_AddRefs (target));
	nsCOMPtr <nsIDOMNode> node = do_QueryInterface(target);
	
	if (type.Equals(NS_LITERAL_STRING("click")))
		ret = events->OnMouseClick (mouseInfo, modifiers, node);
	else if (type.Equals (NS_LITERAL_STRING("mousedown")))
		ret = events->OnMouseDown (mouseInfo, modifiers, node);
	else if (type.Equals (NS_LITERAL_STRING("mouseup")))
		ret = events->OnMouseUp (mouseInfo, modifiers, node);
	else if (type.Equals (NS_LITERAL_STRING("dblclick")))
		ret = events->OnMouseDoubleClick (mouseInfo, modifiers, node);
	else if (type.Equals (NS_LITERAL_STRING("mouseover")))
		ret = events->OnMouseOver (mouseInfo, modifiers, node);
	else if (type.Equals (NS_LITERAL_STRING("mouseout")))
		ret = events->OnMouseOut (mouseInfo, modifiers, node);
	return PR_FALSE;
}
