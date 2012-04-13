/*
 * BrowserWindow.cpp: The browser window class
 *
 * Author:
 *	Andreia Gaita  <avidigal@novell.com>
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include "interfaces.h"

#include "BrowserWindow.h"
#include "SecurityWarningsDialogs.h"
#include "PromptService.h"

// for getting the native mozilla drawing handle

#ifdef NS_UNIX
#include "gtkWidget.h"
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#endif	


NS_GENERIC_FACTORY_CONSTRUCTOR(SecurityWarningDialogs)
NS_GENERIC_FACTORY_CONSTRUCTOR(PromptService)

BrowserWindow::BrowserWindow (void)
{
	owner = nsnull;
	isChrome = PR_FALSE;
	isLoaded = PR_FALSE;
	isFocused = PR_FALSE;
}

nsresult
BrowserWindow::Shutdown ()
{
	PRINT("BrowserWindow::Shutdown\n");
	if (!webBrowser)
		return NS_OK;

	nsCOMPtr<nsIWebProgressListener> wpl ((nsIWebProgressListener*) this);
    nsCOMPtr<nsIWeakReference> weakWpl (NS_GetWeakReference (wpl));
	if (weakWpl)
		webBrowser->RemoveWebBrowserListener (weakWpl, NS_GET_IID (nsIWebProgressListener));
	listeners.clear ();
	return NS_OK;
}

nsresult
BrowserWindow::Create ( Handle * hwnd, PRInt32 width, PRInt32 height)
{
	PRINT("BrowserWindow::Create\n");
	nsresult result;

	webBrowser = do_CreateInstance( NS_WEBBROWSER_CONTRACTID );
	if ( ! webBrowser )
		return NS_ERROR_FAILURE;

    (void) webBrowser->SetContainerWindow (static_cast<nsIWebBrowserChrome *> (this));

    nsCOMPtr<nsIDocShellTreeItem> dsti = do_QueryInterface( webBrowser );
    dsti->SetItemType( nsIDocShellTreeItem::typeContentWrapper );

	webNav = do_QueryInterface(webBrowser);
	sessionHistory = do_CreateInstance(NS_SHISTORY_CONTRACTID);
	webNav->SetSessionHistory(sessionHistory);

    nsCOMPtr<nsIWindowCreator> windowCreator (static_cast<nsIWindowCreator *> (this));

    // Attach it via the watcher service
    nsCOMPtr<nsIWindowWatcher> watcher = do_GetService(NS_WINDOWWATCHER_CONTRACTID);
    if (watcher)
      watcher->SetWindowCreator(windowCreator);

	
    nsCOMPtr<nsIX509CertDB> certdb = do_GetService(NS_X509CERTDB_CONTRACTID, &result);

	/** Component registration... ***/
	RegisterComponents ();

    baseWindow = do_QueryInterface (webBrowser);
	
	result = baseWindow->InitWindow( hwnd, nsnull,  0, 0, width, height );
	if (!NS_SUCCEEDED(result)) {
		SHOUT("BrowserWindow: Failed to initialize window\n");
		return NS_ERROR_FAILURE;
	}
    result = baseWindow->Create();
	if (!NS_SUCCEEDED(result)) {
		SHOUT("BrowserWindow: Failed to create window\n");
		return NS_ERROR_FAILURE;
	}

    nsCOMPtr<nsIWebProgressListener> wpl (static_cast<nsIWebProgressListener*>(this));
    nsCOMPtr<nsIWeakReference> weakWpl (NS_GetWeakReference (wpl));
    webBrowser->AddWebBrowserListener (weakWpl, NS_GET_IID (nsIWebProgressListener));

	baseWindow->SetVisibility( PR_TRUE );

	webNav = do_QueryInterface( webBrowser, &result );
	if ( NS_FAILED( result ) || ! webNav ) {
	    return NS_ERROR_FAILURE;
	}


    if ( webBrowser ) {
		// I really hope we don't need this, it calls in nsIWidget.h which calls
		// in a bunch of local includes we don't want
/*		
#ifdef NS_UNIX
		// save the window id of the newly created window
		nsCOMPtr<nsIWidget> mozWidget;
		baseWindow->GetMainWidget(getter_AddRefs(mozWidget));

		GdkWindow *tmp_window = static_cast<GdkWindow *> (mozWidget->GetNativeData(NS_NATIVE_WINDOW));

		// and, thanks to superwin we actually need the parent of that.
		tmp_window = gdk_window_get_parent(tmp_window);
		
		// save the widget ID - it should be the mozarea of the window.
		gpointer data = nsnull;
		gdk_window_get_user_data(tmp_window, &data);
		this->nativeMozWidget = static_cast<Handle *> (data);
#endif		
*/
		return NS_OK;
    }

    return NS_ERROR_FAILURE;
}

// Initialization
nsresult BrowserWindow::RegisterComponents ()
{
	PRINT("BrowserWindow::RegisterComponents\n");
	nsCOMPtr<nsIComponentRegistrar> compReg;
	nsresult rv = NS_GetComponentRegistrar(getter_AddRefs(compReg));
	NS_ENSURE_SUCCESS(rv, rv);

    nsCOMPtr<nsIFactory> componentFactory;
    rv = NS_NewSecurityWarningServiceFactory(getter_AddRefs(componentFactory));
    if (NS_SUCCEEDED(rv)) {
		compReg->RegisterFactory(kSECURITYWARNINGSDIALOGS_CID,
                                 SECURITYWARNINGSDIALOGS_CLASSNAME,
                                 NS_SECURITYWARNINGDIALOGS_CONTRACTID,
                                 componentFactory);
	}

    rv = NS_NewPromptServiceFactory(getter_AddRefs(componentFactory), this);
    if (NS_SUCCEEDED(rv)) {
        compReg->RegisterFactory(kPROMPTSERVICE_CID,
                                 PROMPTSERVICE_CLASSNAME,
                                 NS_PROMPTSERVICE_CONTRACTID,
                                 componentFactory);
	}

	return NS_OK;
}


// Layout
nsresult BrowserWindow::Focus ()
{
	PRINT2("BrowserWindow::Focus - isFocused=%d\n",isFocused);
	if (!isFocused) {
		isFocused = PR_TRUE;
		SetFocus ();
	}
	return NS_OK;
}

nsresult BrowserWindow::Resize (PRUint32 width, PRUint32 height)
{
	return NS_OK;
}

// Navigate
nsresult BrowserWindow::Navigate (nsString uri)
{
	PRINT("BrowserWindow::Navigate\n");

	this->uri = uri;
	return this->Navigate ();
}

nsresult BrowserWindow::Navigate ()
{
	PRINT("BrowserWindow::Navigate2\n");

	if (webNav) {
		if (uri.Length()) {
			webNav->LoadURI( (const PRUnichar*)uri.get(),
				nsIWebNavigation::LOAD_FLAGS_NONE,
					nsnull, nsnull, nsnull );
		}
		return NS_OK;
	};

	return NS_OK;
}

nsresult 
BrowserWindow::Forward ()
{
	PRINT("BrowserWindow::Forward\n");

	if (webNav)	{
		bool/* PRBool */ canGoForward = PR_FALSE;
		webNav->GetCanGoForward(&canGoForward);
		if (canGoForward)
			webNav->GoForward ();
		return canGoForward;
	}
	return PR_FALSE;
}

nsresult 
BrowserWindow::Back ()
{
	PRINT("BrowserWindow::Back\n");

	if (webNav) {
		bool/* PRBool */ canGoBack = PR_FALSE;
		webNav->GetCanGoBack(&canGoBack);
		if (canGoBack)
			webNav->GoBack ();
		return canGoBack;
	}
	return PR_FALSE;
}

nsresult 
BrowserWindow::Home ()
{
	PRINT("BrowserWindow::Forward\n");

	//if (webNav)
	//{
	//	return webNav-> ();
	//}
	return NS_ERROR_FAILURE;
}

nsresult BrowserWindow::Stop ()
{
	PRINT("BrowserWindow::Stop\n");

	if (webNav)
	{
		return webNav->Stop (nsIWebNavigation::STOP_ALL);
	}
	return NS_ERROR_FAILURE;
}

nsresult BrowserWindow::Reload (ReloadOption option)
{
	PRINT("BrowserWindow::Reload\n");

	if (webNav) {
		switch (option) {
			case RELOAD_NONE:
				return webNav->Reload (nsIWebNavigation::LOAD_FLAGS_NONE);
				break;
			case RELOAD_PROXY:
				return webNav->Reload (nsIWebNavigation::LOAD_FLAGS_BYPASS_CACHE);
				break;
			case RELOAD_FULL:
				return webNav->Reload (nsIWebNavigation::LOAD_FLAGS_BYPASS_PROXY);
				break;
		}
	}
	return NS_ERROR_FAILURE;
}

// layout

nsresult BrowserWindow::Show ()
{
	baseWindow = do_QueryInterface ( webBrowser );
	baseWindow->SetVisibility ( PR_TRUE );
	return NS_OK;
}


// Events
nsresult BrowserWindow::AttachEvent (nsIDOMEventTarget * target, const char * type, const char * name) 
{
	nsEmbedCString typeName (type);
	typeName.Append (":");
	typeName.Append (name);
	const char * string = typeName.get();
	listeners [string] = new EventListener ();
	listeners [string]->target = target;
	listeners [string]->owner = this;
	listeners [string]->events = owner->events;
	nsresult rv = target->AddEventListener (NS_ConvertUTF8toUTF16 (name, strlen (name)), listeners[string], PR_TRUE);
	return rv;
}

nsresult BrowserWindow::DetachEvent (const char * type, const char * name) 
{
	nsEmbedCString typeName (type);
	typeName.Append (":");
	typeName.Append (name);
	const char * string = typeName.get();
	if (listeners[string] != nsnull) {
		nsresult rv = listeners[string]->target->RemoveEventListener (NS_ConvertUTF8toUTF16 (name, strlen (name)), listeners[string], PR_TRUE);
		listeners.erase (string);
		return rv;
	}
	return NS_OK;
}

////////////////////////////////////////////////////////////////////////////////
//

NS_IMPL_ADDREF(BrowserWindow)
NS_IMPL_RELEASE(BrowserWindow)
NS_INTERFACE_MAP_BEGIN(BrowserWindow)
	NS_INTERFACE_MAP_ENTRY(nsIWebBrowserChrome)
	NS_INTERFACE_MAP_ENTRY(nsIWebBrowserChromeFocus)
	NS_INTERFACE_MAP_ENTRY(nsIInterfaceRequestor)
	NS_INTERFACE_MAP_ENTRY(nsIWebProgressListener)
//	NS_INTERFACE_MAP_ENTRY(nsIURIContentListener)
	NS_INTERFACE_MAP_ENTRY(nsSupportsWeakReference)
	NS_INTERFACE_MAP_ENTRY_AMBIGUOUS(nsISupports, nsIWebProgressListener)
	NS_INTERFACE_MAP_ENTRY(nsIWindowCreator)
	NS_INTERFACE_MAP_ENTRY(nsIEmbeddingSiteWindow)
	NS_INTERFACE_MAP_ENTRY(nsIEmbeddingSiteWindow2)
	NS_INTERFACE_MAP_ENTRY(nsIContextMenuListener)
NS_INTERFACE_MAP_END

////////////////////////////////////////////////////////////////////////////////


/* void getInterface (in nsIIDRef uuid, [iid_is (uuid), retval] out nsQIResult result); */
NS_IMETHODIMP 
BrowserWindow::GetInterface(const nsIID & aIID, void * *aInstancePtr)
{
	if ( aIID.Equals( NS_GET_IID( nsIDOMWindow ) ) ) {
		if ( webBrowser ) {
			return webBrowser->GetContentDOMWindow( ( nsIDOMWindow** )aInstancePtr );
		};

		return NS_ERROR_NOT_INITIALIZED;
	};

	return QueryInterface( aIID, aInstancePtr );
}


/* void setStatus (in unsigned long statusType, in wstring status); */
NS_IMETHODIMP 
BrowserWindow::SetStatus(PRUint32 statusType, const PRUnichar *status)
{
	owner->events->OnStatusChange (NULL, NULL, status, 100);
	return NS_OK;
}

/* attribute nsIWebBrowser webBrowser; */
NS_IMETHODIMP 
BrowserWindow::GetWebBrowser(nsIWebBrowser * *aWebBrowser)
{
	//NS_ENSURE_ARG_POINTER( aWebBrowser );
	*aWebBrowser = webBrowser;
	NS_IF_ADDREF( *aWebBrowser );
	return NS_OK;
}
NS_IMETHODIMP 
BrowserWindow::SetWebBrowser(nsIWebBrowser * aWebBrowser)
{
	NS_ENSURE_ARG_POINTER( aWebBrowser );
	webBrowser = aWebBrowser;
	return NS_OK;
}

NS_IMETHODIMP 
BrowserWindow::CreateChromeWindow(
			nsIWebBrowserChrome *aParent,
			PRUint32 aChromeFlags,
			nsIWebBrowserChrome **_retval)
{

//	if (!owner->events->EventCreateNewWindow ())
//		return NS_ERROR_FAILURE;

	//if (aChromeFlags & nsIWebBrowserChrome::CHROME_OPENAS_CHROME)
	//	this->isChrome = PR_TRUE;

	*_retval = static_cast<nsIWebBrowserChrome *> (this);
	
	if (*_retval) {
		NS_ADDREF(*_retval);
		return NS_OK;
	}

#ifdef NS_UNIX
	PRINT("BrowserWindow::CreateChromeWindow\n");
	NS_ENSURE_ARG_POINTER(_retval);

	NativeEmbedWidget *newEmbed = nsnull;
   
    if (!this->owner)
      return NS_ERROR_FAILURE;
       

	// check to make sure that we made a new window
	if (!newEmbed)
		return NS_ERROR_FAILURE;
	
	// The window _must_ be realized before we pass it back to the
	// function that created it. Functions that create new windows
	// will do things like GetDocShell() and the widget has to be
	// realized before that can happen.
	gtk_widget_realize(GTK_WIDGET(newEmbed));
	
	Widget *newOwner = static_cast<Widget *> (newEmbed->data);
	
	// set the chrome flag on the new window if it's a chrome open
//	if (aChromeFlags & nsIWebBrowserChrome::CHROME_OPENAS_CHROME)
//		newOwner->mIsChrome = PR_TRUE;
	
	*_retval = static_cast<nsIWebBrowserChrome *> (this);
	
	if (*_retval) {
		NS_ADDREF(*_retval);
		return NS_OK;
	}
	
	return NS_ERROR_FAILURE;
#endif

	return NS_OK;
}

/* void onStateChange (in nsIWebProgress aWebProgress, in nsIRequest aRequest, in unsigned long aStateFlags, in nsresult aStatus); */
NS_IMETHODIMP 
BrowserWindow::OnStateChange(nsIWebProgress* progress, nsIRequest* request,
										   PRUint32 state, nsresult status)
{	
	owner->events->OnStateChange(progress, request, status, state);
	
	bool netstop = ( state & STATE_STOP ) && ( state & STATE_IS_NETWORK ) && ( status == NS_OK );
	bool windowstop = ( state & STATE_STOP ) && ( state & STATE_IS_WINDOW ) && ( status == NS_OK );
	
	if (netstop) {
		bool/* PRBool */ visibility;
		this->GetVisibility(&visibility);
		if (visibility)
			this->SetVisibility(PR_TRUE);
	}
	
	if ( windowstop ) {
		nsCOMPtr< nsIDOMWindow > window;
		nsresult result = progress->GetDOMWindow( getter_AddRefs( window ) );
		nsCOMPtr< nsIDOMEventTarget > target = do_QueryInterface( window );
		AttachEvent ( target, "window", "load" );
		AttachEvent ( target, "window", "unload" );
#ifndef NS_WIN32
		AttachEvent ( target, "window", "focus" );
#endif
		AttachEvent ( target, "window", "blur" );
		AttachEvent ( target, "window", "abort" );
		AttachEvent ( target, "window", "error" );
		AttachEvent ( target, "window", "activate" );
		AttachEvent ( target, "window", "deactivate" );
		AttachEvent ( target, "window", "focusin" );
		AttachEvent ( target, "window", "focusout" );		

		AttachEvent ( target, "window", "input" );
		AttachEvent ( target, "window", "select" );
		AttachEvent ( target, "window", "change" );
		AttachEvent ( target, "window", "submit" );
		AttachEvent ( target, "window", "reset" );
		
		AttachEvent ( target, "window", "keydown" );
		AttachEvent ( target, "window", "keypress" );
		AttachEvent ( target, "window", "keyup" );

		AttachEvent ( target, "window", "click" );
		AttachEvent ( target, "window", "dblclick" );
		AttachEvent ( target, "window", "mousedown" );
		AttachEvent ( target, "window", "mouseup" );
		AttachEvent ( target, "window", "mouseover" );
		AttachEvent ( target, "window", "mouseout" );
		AttachEvent ( target, "window", "mousemove" );
		
		AttachEvent ( target, "window", "popupshowing" );
		AttachEvent ( target, "window", "popupshown" );
		AttachEvent ( target, "window", "popuphiding" );
		AttachEvent ( target, "window", "popuphidden" );
		
	}
	return NS_OK;
}

// TODO


/* attribute unsigned long chromeFlags; */
NS_IMETHODIMP 
BrowserWindow::GetChromeFlags(PRUint32 *aChromeFlags)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP 
BrowserWindow::SetChromeFlags(PRUint32 aChromeFlags)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void destroyBrowserWindow (); */
NS_IMETHODIMP 
BrowserWindow::DestroyBrowserWindow()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void sizeBrowserTo (in long aCX, in long aCY); */
NS_IMETHODIMP 
BrowserWindow::SizeBrowserTo(PRInt32 aCX, PRInt32 aCY)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void showAsModal (); */
NS_IMETHODIMP BrowserWindow::ShowAsModal()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* boolean isWindowModal (); */
NS_IMETHODIMP 
BrowserWindow::IsWindowModal(bool/* PRBool */ *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void exitModalEventLoop (in nsresult aStatus); */
NS_IMETHODIMP 
BrowserWindow::ExitModalEventLoop(nsresult aStatus)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void onProgressChange (in nsIWebProgress aWebProgress, in nsIRequest aRequest, in long aCurSelfProgress, in long aMaxSelfProgress, in long aCurTotalProgress, in long aMaxTotalProgress); */
NS_IMETHODIMP 
BrowserWindow::OnProgressChange(nsIWebProgress *webProgress, nsIRequest *request, PRInt32 aCurSelfProgress, PRInt32 aMaxSelfProgress, PRInt32 aCurTotalProgress, PRInt32 aMaxTotalProgress)
{	
	owner->events->OnProgress(webProgress, request, aCurTotalProgress, aMaxTotalProgress);
    return NS_OK;
}

/* void onLocationChange (in nsIWebProgress aWebProgress, in nsIRequest aRequest, in nsIURI aLocation); */
#if 1
NS_IMETHODIMP 
BrowserWindow::OnLocationChange(nsIWebProgress *webProgress, nsIRequest *request, nsIURI *location, PRUint32 foo)
#else
NS_IMETHODIMP 
BrowserWindow::OnLocationChange(nsIWebProgress *webProgress, nsIRequest *request, nsIURI *location)
#endif
{
    owner->events->OnLocationChanged(webProgress, request, location);
	return NS_OK;
}

/* void onStatusChange (in nsIWebProgress aWebProgress, in nsIRequest aRequest, in nsresult aStatus, in wstring aMessage); */
NS_IMETHODIMP 
BrowserWindow::OnStatusChange(nsIWebProgress *webProgress, nsIRequest *request, nsresult aStatus, const PRUnichar *aMessage)
{
	PRINT ("gluezilla: OnStatusChange\n");
	owner->events->OnStatusChange (webProgress, request, aMessage, aStatus);
    return NS_OK;
}

/* void onSecurityChange (in nsIWebProgress aWebProgress, in nsIRequest aRequest, in unsigned long aState); */
NS_IMETHODIMP 
BrowserWindow::OnSecurityChange (nsIWebProgress *webProgress, nsIRequest *request, PRUint32 state)
{
    owner->events->OnSecurityChange (webProgress, request, state);
	return NS_OK;
}


/* nsIURIContentListener */

/* boolean onStartURIOpen (in nsIURI aURI); */
//NS_IMETHODIMP 
//BrowserWindow::OnStartURIOpen(nsIURI *aURI, bool/* PRBool */ *_retval)
//{
//    return NS_ERROR_NOT_IMPLEMENTED;
//}

/* boolean doContent (in string aContentType, in boolean aIsContentPreferred, in nsIRequest aRequest, out nsIStreamListener aContentHandler); */
//NS_IMETHODIMP 
//BrowserWindow::DoContent(const char *aContentType, bool/* PRBool */ aIsContentPreferred, nsIRequest *aRequest, nsIStreamListener **aContentHandler, bool/* PRBool */ *_retval)
//{
//    return NS_ERROR_NOT_IMPLEMENTED;
//}

/* boolean isPreferred (in string aContentType, out string aDesiredContentType); */
//NS_IMETHODIMP 
//BrowserWindow::IsPreferred(const char *aContentType, char **aDesiredContentType, bool/* PRBool */ *_retval)
//{
//    return NS_ERROR_NOT_IMPLEMENTED;
//}

/* boolean canHandleContent (in string aContentType, in boolean aIsContentPreferred, out string aDesiredContentType); */
//NS_IMETHODIMP 
//BrowserWindow::CanHandleContent(const char *aContentType, bool/* PRBool */ aIsContentPreferred, char **aDesiredContentType, bool/* PRBool */ *_retval)
//{
//    return NS_ERROR_NOT_IMPLEMENTED;
//}

/* attribute nsISupports loadCookie; */
//NS_IMETHODIMP 
//BrowserWindow::GetLoadCookie(nsISupports * *aLoadCookie)
//{
//    return NS_ERROR_NOT_IMPLEMENTED;
//}
//NS_IMETHODIMP 
//BrowserWindow::SetLoadCookie(nsISupports * aLoadCookie)
//{
//    return NS_ERROR_NOT_IMPLEMENTED;
//}

/* attribute nsIURIContentListener parentContentListener; */
//NS_IMETHODIMP 
//BrowserWindow::GetParentContentListener(nsIURIContentListener * *aParentContentListener)
//{
//    return NS_ERROR_NOT_IMPLEMENTED;
//}
//NS_IMETHODIMP 
//BrowserWindow::SetParentContentListener(nsIURIContentListener * aParentContentListener)
//{
//    return NS_ERROR_NOT_IMPLEMENTED;
//}



// nsIEmbeddingSiteWindow

/* void setDimensions (in unsigned long flags, in long x, in long y, in long cx, in long cy); */
NS_IMETHODIMP 
BrowserWindow::SetDimensions(PRUint32 flags, 
                             PRInt32 x, PRInt32 y, 
                             PRInt32 cx, PRInt32 cy)
{
	if (flags & nsIEmbeddingSiteWindow::DIM_FLAGS_POSITION &&
			(flags & (nsIEmbeddingSiteWindow::DIM_FLAGS_SIZE_INNER |
			nsIEmbeddingSiteWindow::DIM_FLAGS_SIZE_OUTER))) {
		return baseWindow->SetPositionAndSize(x, y, cx, cy, PR_TRUE);
	}
	else if (flags & nsIEmbeddingSiteWindow::DIM_FLAGS_POSITION) {
		return baseWindow->SetPosition(x, y);
	}
	else if (flags & (nsIEmbeddingSiteWindow::DIM_FLAGS_SIZE_INNER |
			nsIEmbeddingSiteWindow::DIM_FLAGS_SIZE_OUTER)) {
		return baseWindow->SetSize(cx, cy, PR_TRUE);
	}
	return NS_ERROR_INVALID_ARG;
}

/* void getDimensions (in unsigned long flags, out long x, out long y, out long cx, out long cy); */
NS_IMETHODIMP 
BrowserWindow::GetDimensions(PRUint32 flags, PRInt32 *x, PRInt32 *y, PRInt32 *cx, PRInt32 *cy)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void setFocus (); */
NS_IMETHODIMP 
BrowserWindow::SetFocus()
{
    return baseWindow->SetFocus();
}

// nsIEmbeddingSiteWindow2
NS_IMETHODIMP 
BrowserWindow::Blur()
{
    return NS_OK;
}

/* attribute boolean visibility; */
NS_IMETHODIMP 
BrowserWindow::GetVisibility(bool/* PRBool */ *aVisibility)
{
//	baseWindow->GetVisibility (aVisibility);
	return NS_OK;
}
NS_IMETHODIMP 
BrowserWindow::SetVisibility(bool/* PRBool */ aVisibility)
{
//	baseWindow->SetVisibility (aVisibility);
	return NS_OK;
}

/* attribute wstring title; */
NS_IMETHODIMP BrowserWindow::GetTitle(PRUnichar * *aTitle)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP 
BrowserWindow::SetTitle(const PRUnichar * aTitle)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* [noscript] readonly attribute voidPtr siteWindow; */
NS_IMETHODIMP 
BrowserWindow::GetSiteWindow(void * *aSiteWindow)
{
  *aSiteWindow = static_cast<void *> (owner->getHandle());
  return NS_OK;
}


// nsIWebBrowserChromeFocus
/* void focusNextElement (); */
NS_IMETHODIMP 
BrowserWindow::FocusNextElement()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void focusPrevElement (); */
NS_IMETHODIMP 
BrowserWindow::FocusPrevElement()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

// nsIContextMenuListener

NS_IMETHODIMP 
BrowserWindow::OnShowContextMenu (PRUint32 contextFlags, nsIDOMEvent * event, nsIDOMNode * node)
{
	PRINT ("gluezilla: OnShowContextMenu\n");
	owner->events->OnShowContextMenu (contextFlags, event, node);
	return NS_OK;
}


