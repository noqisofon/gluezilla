/* gluezilla.cpp: external interface
 *
 * Author:
 *	Andreia Gaita  <avidigal@novell.com>
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include "gluezilla.h"
#include "Widget.h"
#include <stdlib.h>

#ifdef NS_UNIX
#include "gtkWidget.h"
GThread    *ui_thread_id;
GAsyncQueue *queuein;
GAsyncQueue *queueout;
#endif

NS_METHOD_(void)
gluezilla_debug (int signal)
{
	PRINT2 ("debug signal: %d\n", signal);
}

NS_METHOD_(short)
gluezilla_init (Platform platform, Platform * mozPlatform)
{	
#ifdef NS_UNIX
	if (platform == Winforms) {
		gtk_initialize_thread ();
	}
	*mozPlatform = Gtk;
#endif
#ifdef NS_WIN32
	*mozPlatform = Winforms;
#endif

#if XUL_VERSION == 2
	return 2;
#else
	return 3;
#endif
}

NS_METHOD_(Handle*)
gluezilla_bind (CallbackBin *events, Handle *hwnd, 
			    PRInt32 width, PRInt32 height, 
			    const char * startDir, const char * dataDir, 
			    Platform platform)
{
	Widget *widget = new Widget (strdup(startDir), strdup(dataDir), platform);
	
	Params * p = new Params ();
	p->name = "init";
	p->instance = widget;
	p->events = events;

	nsresult result = widget->BeginInvoke (p);
	if (p)
		delete (p);

	if (!NS_SUCCEEDED(result))
		return NULL;

	Handle * handle = hwnd;

	p = new Params ();
	p->name = "bind";
	p->instance = widget;
	p->hwnd = hwnd;
	p->width = width;
	p->height = height;

	result = widget->BeginInvoke (p);
	if (p) {
		p->name = NULL;
		p->instance = NULL;
		p->hwnd = NULL;
		delete (p);
	}

	if (!NS_SUCCEEDED(result))
		return NULL;

	return reinterpret_cast<Handle*>(widget);
}

NS_METHOD_(int) gluezilla_createBrowserWindow (Handle *instance)
{
	Widget *widget = reinterpret_cast<Widget *> (instance);

	Params * p = new Params ();
	p->name = "create";
	p->instance = widget;

	nsresult result = widget->BeginInvoke (p);
	if (p) {
		p->name = NULL;
		p->instance = NULL;
		delete (p);
	}
	return result;
}

/*******************
Layout
*******************/
NS_METHOD_(int) gluezilla_focus (Handle *instance, FocusOption focus)
{
	Widget *widget = reinterpret_cast<Widget *> (instance);

	Params * p = new Params ();
	p->name = "focus";
	p->instance = widget;
	p->focus = focus;

	nsresult result = widget->BeginInvoke (p);
	if (p) {
		p->name = NULL;
		p->instance = NULL;
		delete (p);
	}
	return result;
}

NS_METHOD_(int) gluezilla_blur (Handle *instance)
{
	Widget *widget = reinterpret_cast<Widget *> (instance);

	Params * p = new Params ();
	p->name = "blur";
	p->instance = widget;

	nsresult result = widget->BeginInvoke (p);
	if (p) {
		p->name = NULL;
		p->instance = NULL;
		delete (p);
	}
	return result;
}

NS_METHOD_(int) gluezilla_activate (Handle *instance)
{
	Widget *widget = reinterpret_cast<Widget *> (instance);

	Params * p = new Params ();
	p->name = "activate";
	p->instance = widget;

	nsresult result = widget->BeginInvoke (p);
	if (p) {
		p->name = NULL;
		p->instance = NULL;
		delete (p);
	}
	return result;
}

NS_METHOD_(int) gluezilla_deactivate (Handle *instance)
{
	Widget *widget = reinterpret_cast<Widget *> (instance);

	Params * p = new Params ();
	p->name = "deactivate";
	p->instance = widget;

	nsresult result = widget->BeginInvoke (p);
	if (p) {
		p->name = NULL;
		p->instance = NULL;
		delete (p);
	}
	return result;
}

NS_METHOD_(int) gluezilla_resize (Handle *instance, PRUint32 width, PRUint32 height)
{
	Widget *widget = reinterpret_cast<Widget *> (instance);

	Params * p = new Params ();
	p->name = "resize";
	p->instance = widget;
	p->width = width;
	p->height = height;

	nsresult result = widget->BeginInvoke (p);
	if (p) {
		p->name = NULL;
		p->instance = NULL;
		delete (p);
	}
	return result;
}

/*******************
Navigation
*******************/
NS_METHOD_(int)
gluezilla_navigate (Handle *instance, const char * uri)
{
	Widget *widget = reinterpret_cast<Widget *> (instance);
	
	Params * p = new Params ();
	p->name = "navigate";
	p->instance = widget;
	p->string = strdup (uri);

	nsresult result = widget->BeginInvoke (p);
	if (p) {
		p->name = NULL;
		p->instance = NULL;
		free (p->string);
		p->string = NULL;
		delete (p);
	}
	return result;
}

NS_METHOD_(bool/* PRBool */)
gluezilla_forward (Handle *instance)
{
	Widget *widget = reinterpret_cast<Widget *> (instance);
	
	Params * p = new Params ();
	p->name = "forward";
	p->instance = widget;

	nsresult result = widget->BeginInvoke (p);
	if (p) {
		p->name = NULL;
		p->instance = NULL;
		delete (p);
	}
	return result;
}

NS_METHOD_(bool/* PRBool */)
gluezilla_back (Handle *instance)
{
	Widget *widget = reinterpret_cast<Widget *> (instance);
	
	Params * p = new Params ();
	p->name = "back";
	p->instance = widget;

	nsresult result = widget->BeginInvoke (p);
	if (p) {
		p->name = NULL;
		p->instance = NULL;
		delete (p);
	}
	return result;
}

NS_METHOD_(int)
gluezilla_home (Handle *instance)
{
	Widget *widget = reinterpret_cast<Widget *> (instance);
	
	Params * p = new Params ();
	p->name = "home";
	p->instance = widget;

	nsresult result = widget->BeginInvoke (p);
	if (p) {
		p->name = NULL;
		p->instance = NULL;
		delete (p);
	}
	return result;
}

NS_METHOD_(int)
gluezilla_stop (Handle *instance)
{
	Widget *widget = reinterpret_cast<Widget *> (instance);
	
	Params * p = new Params ();
	p->name = "stop";
	p->instance = widget;

	nsresult result = widget->BeginInvoke (p);
	if (p) {
		p->name = NULL;
		p->instance = NULL;
		delete (p);
	}
	return result;
}

NS_METHOD_(int)
gluezilla_reload (Handle *instance, ReloadOption option)
{
	Widget *widget = reinterpret_cast<Widget *> (instance);
	
	Params * p = new Params ();
	p->name = "reload";
	p->instance = widget;
	p->option = option;

	nsresult result = widget->BeginInvoke (p);
	if (p) {
		p->name = NULL;
		p->instance = NULL;
		delete (p);
	}
	return result;
}

NS_METHOD_(int)
gluezilla_shutdown (Handle *instance)
{
	Widget *widget = reinterpret_cast<Widget *> (instance);

	Params * p = new Params ();
	p->name = "shutdown";
	p->instance = widget;

	nsresult result = widget->BeginInvoke (p);
	if (p) {
		p->name = NULL;
		p->instance = NULL;
		delete (p);
	}
	return result;	
}


NS_METHOD_(nsIDOMHTMLDocument*)
gluezilla_getDomDocument (Handle *instance)
{
	Widget *widget = reinterpret_cast<Widget *> (instance);

	Params * p = new Params ();
	p->name = "getDocument";
	p->instance = widget;

	nsresult result = widget->BeginInvoke (p);
	nsIDOMHTMLDocument * ret (p->document);
	NS_ADDREF(ret);
	if (p) {
		p->name = NULL;
		p->instance = NULL;
		delete (p);
	}
	return ret;
}

NS_METHOD_(nsIWebNavigation*)
gluezilla_getWebNavigation (Handle *instance)
{
	Widget * widget = reinterpret_cast<Widget *> (instance);
	Params * p = new Params ();
	p->name = "getNavigation";
	p->instance = widget;

	nsresult result = widget->BeginInvoke (p);
	nsIWebNavigation * ret (p->navigation);
	NS_ADDREF(ret);
	if (p) {
		p->name = NULL;
		p->instance = NULL;
		delete (p);
	}
	return ret;
}

NS_METHOD_(void)
gluezilla_setString (Handle *instance, nsString & ret)
{
	Widget *widget = reinterpret_cast<Widget *> (instance);
	nsCOMPtr<nsIDOMWindow> domWindow;
	
	widget->browserWindow->webBrowser->GetContentDOMWindow( getter_AddRefs (domWindow) );
	nsCOMPtr<nsIDOMDocument> domDoc;
	domWindow->GetDocument (getter_AddRefs(domDoc));
	nsCOMPtr<nsIDOMNode> node;
	domDoc->GetFirstChild (getter_AddRefs (node));
	node->GetLocalName (ret);

	const PRUnichar * s = (PRUnichar *)ret.get ();
	
}

NS_METHOD_(nsString *)
gluezilla_stringInit()
{
	return new nsString ();
}

NS_METHOD_(nsresult)
gluezilla_stringFinish(nsString * string)
{
	NS_StringContainerFinish (reinterpret_cast<nsStringContainer&> (*string));
	delete string;
	return NS_OK;
}

NS_METHOD_(PRUnichar*)
gluezilla_stringGet(nsString & str)
{
	return (PRUnichar *)str.get ();
}

NS_METHOD_(void)
gluezilla_stringSet(nsString & str, PRUnichar * text)
{
	str.Assign (text);
}

NS_METHOD_(nsIServiceManager*)
gluezilla_getServiceManager()
{
	nsCOMPtr<nsIServiceManager> servMan;
	NS_GetServiceManager (getter_AddRefs (servMan));
	return servMan;
}

NS_METHOD_(nsIServiceManager*)
gluezilla_getServiceManager2 (Handle *instance)
{
	Widget *widget = reinterpret_cast<Widget *> (instance);
	Params * p = new Params ();
	p->name = "getServiceManager";
	p->instance = widget;
	nsresult rv = widget->BeginInvoke (p);
	nsIServiceManager * ret (reinterpret_cast<nsIServiceManager *> (p->result));
	NS_ADDREF(ret);
	if (p) {
		p->name = NULL;
		p->instance = NULL;
		delete (p);
	}
	
	return ret;
}

NS_METHOD_(void)
gluezilla_getProxyForObject (Handle *instance, REFNSIID iid, nsISupports *object, nsISupports ** result)
{
	Widget *widget = reinterpret_cast<Widget *> (instance);

	Params * p = new Params ();
	p->name = "getProxyForObject";
	p->instance = widget;
	p->object = object;
	p->iid = iid;

	nsresult rv = widget->BeginInvoke (p);

	NS_ADDREF(*result = p->result);
	NS_RELEASE (p->result);

	if (p) {
		p->name = NULL;
		p->instance = NULL;
		p->object = NULL;
		delete (p);
	}
	return;
}


NS_METHOD_(nsresult)
gluezilla_StringContainerInit (nsStringContainer &aStr)
{
	return NS_StringContainerInit (aStr);
}

NS_METHOD_(void)
gluezilla_StringContainerFinish (nsStringContainer &aStr)
{
	NS_StringContainerFinish (aStr);
}

NS_METHOD_(PRUint32)
gluezilla_StringGetData (const nsAString &aStr, const PRUnichar **aBuf, bool/* PRBool */ *aTerm)
{
	return NS_StringGetData (aStr, aBuf, aTerm);
}

NS_METHOD_(nsresult)
gluezilla_StringSetData (nsAString &aStr, const PRUnichar *aBuf, PRUint32 aCount)
{
	return NS_StringSetData (aStr, aBuf, aCount);
}
	
NS_METHOD_(nsresult)
gluezilla_CStringContainerInit (nsCStringContainer &aStr)
{
	return NS_CStringContainerInit (aStr);
}

NS_METHOD_(void)
gluezilla_CStringContainerFinish (nsCStringContainer &aStr)
{
	NS_CStringContainerFinish (aStr);
}

NS_METHOD_(PRUint32)
gluezilla_CStringGetData (const nsACString &aStr, const char **aBuf, bool/* PRBool */ *aTerm)
{
	return NS_CStringGetData (aStr, aBuf, aTerm);
}

NS_METHOD_(nsresult)
gluezilla_CStringSetData (nsACString &aStr, const char *aBuf, PRUint32 aCount)
{
	return NS_CStringSetData (aStr, aBuf, aCount);
}

NS_METHOD_(PRUnichar *)
gluezilla_evalScript (Handle *instance, const char * script)
{
	Widget *widget = reinterpret_cast<Widget *> (instance);	
	Params * p = new Params ();
	p->name = "evalScript";
	p->instance = widget;
	p->string = strdup (script);

	nsresult result = widget->BeginInvoke (p, FALSE);
	if (NS_FAILED(result))
		return NULL;

	PRUnichar * string = p->uniString;
	if (p) {
		free (p->string);
		p->instance = NULL;
		p->name = NULL;
		delete (p);
	}
	return string;
}


#ifdef NS_UNIX
void gtk_initialize_thread () 
{
	g_type_init();	
	if (!g_thread_supported ()) g_thread_init (NULL);

	queuein = g_async_queue_new ();
	queueout = g_async_queue_new ();

	ui_thread_id = g_thread_create (gtk_startup, NULL, TRUE, NULL);
	g_async_queue_pop (queueout);
}

void *
gtk_startup (gpointer data)
{
	PRINT2 ("wakeup_gtk %p starting...\n", g_thread_self ());
	gdk_threads_enter ();	

	int argc = 0;
	char **argv=NULL;
	gtk_init(&argc, &argv);
	g_idle_add (gtk_init_done, NULL);
	gtk_main();
	gdk_threads_leave ();

	return NULL;
}

gboolean
gtk_init_done (gpointer data)
{
	PRINT2 ("callback_initdone %p \n", g_thread_self ());	
	int p = 1;
	g_async_queue_push (queueout, &p);
}

gboolean
gtk_shutdown (gpointer data)
{
	PRINT2 ("gtk_shutdown %p \n", g_thread_self ());	
	gtk_main_quit ();
}
#endif
