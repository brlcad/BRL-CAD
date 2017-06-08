/*
 * tkIntPlatDecls.h --
 *
 *	This file contains the declarations for all platform dependent
 *	unsupported functions that are exported by the Tk library.  These
 *	interfaces are not guaranteed to remain the same between
 *	versions.  Use at your own risk.
 *
 * Copyright (c) 1998-1999 by Scriptics Corporation.
 * All rights reserved.
 */

#ifndef _TKINTPLATDECLS
#define _TKINTPLATDECLS

#ifdef BUILD_tk
#undef TCL_STORAGE_CLASS
#define TCL_STORAGE_CLASS DLLEXPORT
#endif

/*
 * WARNING: This file is automatically generated by the tools/genStubs.tcl
 * script.  Any modifications to the function declarations below should be made
 * in the generic/tkInt.decls script.
 */

/* !BEGIN!: Do not edit below this line. */

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Exported function declarations:
 */

#if defined(__WIN32__) || defined(__CYGWIN__) /* WIN */
#ifndef TkAlignImageData_TCL_DECLARED
#define TkAlignImageData_TCL_DECLARED
/* 0 */
EXTERN char *		TkAlignImageData(XImage *image, int alignment,
				int bitOrder);
#endif
/* Slot 1 is reserved */
#ifndef TkGenerateActivateEvents_TCL_DECLARED
#define TkGenerateActivateEvents_TCL_DECLARED
/* 2 */
EXTERN void		TkGenerateActivateEvents(TkWindow *winPtr,
				int active);
#endif
#ifndef TkpGetMS_TCL_DECLARED
#define TkpGetMS_TCL_DECLARED
/* 3 */
EXTERN unsigned long	TkpGetMS(void);
#endif
#ifndef TkPointerDeadWindow_TCL_DECLARED
#define TkPointerDeadWindow_TCL_DECLARED
/* 4 */
EXTERN void		TkPointerDeadWindow(TkWindow *winPtr);
#endif
#ifndef TkpPrintWindowId_TCL_DECLARED
#define TkpPrintWindowId_TCL_DECLARED
/* 5 */
EXTERN void		TkpPrintWindowId(char *buf, Window window);
#endif
#ifndef TkpScanWindowId_TCL_DECLARED
#define TkpScanWindowId_TCL_DECLARED
/* 6 */
EXTERN int		TkpScanWindowId(Tcl_Interp *interp,
				CONST char *string, Window *idPtr);
#endif
#ifndef TkpSetCapture_TCL_DECLARED
#define TkpSetCapture_TCL_DECLARED
/* 7 */
EXTERN void		TkpSetCapture(TkWindow *winPtr);
#endif
#ifndef TkpSetCursor_TCL_DECLARED
#define TkpSetCursor_TCL_DECLARED
/* 8 */
EXTERN void		TkpSetCursor(TkpCursor cursor);
#endif
#ifndef TkpWmSetState_TCL_DECLARED
#define TkpWmSetState_TCL_DECLARED
/* 9 */
EXTERN int		TkpWmSetState(TkWindow *winPtr, int state);
#endif
#ifndef TkSetPixmapColormap_TCL_DECLARED
#define TkSetPixmapColormap_TCL_DECLARED
/* 10 */
EXTERN void		TkSetPixmapColormap(Pixmap pixmap, Colormap colormap);
#endif
#ifndef TkWinCancelMouseTimer_TCL_DECLARED
#define TkWinCancelMouseTimer_TCL_DECLARED
/* 11 */
EXTERN void		TkWinCancelMouseTimer(void);
#endif
#ifndef TkWinClipboardRender_TCL_DECLARED
#define TkWinClipboardRender_TCL_DECLARED
/* 12 */
EXTERN void		TkWinClipboardRender(TkDisplay *dispPtr, UINT format);
#endif
#ifndef TkWinEmbeddedEventProc_TCL_DECLARED
#define TkWinEmbeddedEventProc_TCL_DECLARED
/* 13 */
EXTERN LRESULT		TkWinEmbeddedEventProc(HWND hwnd, UINT message,
				WPARAM wParam, LPARAM lParam);
#endif
#ifndef TkWinFillRect_TCL_DECLARED
#define TkWinFillRect_TCL_DECLARED
/* 14 */
EXTERN void		TkWinFillRect(HDC dc, int x, int y, int width,
				int height, int pixel);
#endif
#ifndef TkWinGetBorderPixels_TCL_DECLARED
#define TkWinGetBorderPixels_TCL_DECLARED
/* 15 */
EXTERN COLORREF		TkWinGetBorderPixels(Tk_Window tkwin,
				Tk_3DBorder border, int which);
#endif
#ifndef TkWinGetDrawableDC_TCL_DECLARED
#define TkWinGetDrawableDC_TCL_DECLARED
/* 16 */
EXTERN HDC		TkWinGetDrawableDC(Display *display, Drawable d,
				TkWinDCState *state);
#endif
#ifndef TkWinGetModifierState_TCL_DECLARED
#define TkWinGetModifierState_TCL_DECLARED
/* 17 */
EXTERN int		TkWinGetModifierState(void);
#endif
#ifndef TkWinGetSystemPalette_TCL_DECLARED
#define TkWinGetSystemPalette_TCL_DECLARED
/* 18 */
EXTERN HPALETTE		TkWinGetSystemPalette(void);
#endif
#ifndef TkWinGetWrapperWindow_TCL_DECLARED
#define TkWinGetWrapperWindow_TCL_DECLARED
/* 19 */
EXTERN HWND		TkWinGetWrapperWindow(Tk_Window tkwin);
#endif
#ifndef TkWinHandleMenuEvent_TCL_DECLARED
#define TkWinHandleMenuEvent_TCL_DECLARED
/* 20 */
EXTERN int		TkWinHandleMenuEvent(HWND *phwnd, UINT *pMessage,
				WPARAM *pwParam, LPARAM *plParam,
				LRESULT *plResult);
#endif
#ifndef TkWinIndexOfColor_TCL_DECLARED
#define TkWinIndexOfColor_TCL_DECLARED
/* 21 */
EXTERN int		TkWinIndexOfColor(XColor *colorPtr);
#endif
#ifndef TkWinReleaseDrawableDC_TCL_DECLARED
#define TkWinReleaseDrawableDC_TCL_DECLARED
/* 22 */
EXTERN void		TkWinReleaseDrawableDC(Drawable d, HDC hdc,
				TkWinDCState *state);
#endif
#ifndef TkWinResendEvent_TCL_DECLARED
#define TkWinResendEvent_TCL_DECLARED
/* 23 */
EXTERN LRESULT		TkWinResendEvent(WNDPROC wndproc, HWND hwnd,
				XEvent *eventPtr);
#endif
#ifndef TkWinSelectPalette_TCL_DECLARED
#define TkWinSelectPalette_TCL_DECLARED
/* 24 */
EXTERN HPALETTE		TkWinSelectPalette(HDC dc, Colormap colormap);
#endif
#ifndef TkWinSetMenu_TCL_DECLARED
#define TkWinSetMenu_TCL_DECLARED
/* 25 */
EXTERN void		TkWinSetMenu(Tk_Window tkwin, HMENU hMenu);
#endif
#ifndef TkWinSetWindowPos_TCL_DECLARED
#define TkWinSetWindowPos_TCL_DECLARED
/* 26 */
EXTERN void		TkWinSetWindowPos(HWND hwnd, HWND siblingHwnd,
				int pos);
#endif
#ifndef TkWinWmCleanup_TCL_DECLARED
#define TkWinWmCleanup_TCL_DECLARED
/* 27 */
EXTERN void		TkWinWmCleanup(HINSTANCE hInstance);
#endif
#ifndef TkWinXCleanup_TCL_DECLARED
#define TkWinXCleanup_TCL_DECLARED
/* 28 */
EXTERN void		TkWinXCleanup(ClientData clientData);
#endif
#ifndef TkWinXInit_TCL_DECLARED
#define TkWinXInit_TCL_DECLARED
/* 29 */
EXTERN void		TkWinXInit(HINSTANCE hInstance);
#endif
#ifndef TkWinSetForegroundWindow_TCL_DECLARED
#define TkWinSetForegroundWindow_TCL_DECLARED
/* 30 */
EXTERN void		TkWinSetForegroundWindow(TkWindow *winPtr);
#endif
#ifndef TkWinDialogDebug_TCL_DECLARED
#define TkWinDialogDebug_TCL_DECLARED
/* 31 */
EXTERN void		TkWinDialogDebug(int debug);
#endif
#ifndef TkWinGetMenuSystemDefault_TCL_DECLARED
#define TkWinGetMenuSystemDefault_TCL_DECLARED
/* 32 */
EXTERN Tcl_Obj *	TkWinGetMenuSystemDefault(Tk_Window tkwin,
				CONST char *dbName, CONST char *className);
#endif
#ifndef TkWinGetPlatformId_TCL_DECLARED
#define TkWinGetPlatformId_TCL_DECLARED
/* 33 */
EXTERN int		TkWinGetPlatformId(void);
#endif
#ifndef TkWinSetHINSTANCE_TCL_DECLARED
#define TkWinSetHINSTANCE_TCL_DECLARED
/* 34 */
EXTERN void		TkWinSetHINSTANCE(HINSTANCE hInstance);
#endif
#ifndef TkWinGetPlatformTheme_TCL_DECLARED
#define TkWinGetPlatformTheme_TCL_DECLARED
/* 35 */
EXTERN int		TkWinGetPlatformTheme(void);
#endif
#ifndef TkWinChildProc_TCL_DECLARED
#define TkWinChildProc_TCL_DECLARED
/* 36 */
EXTERN LRESULT __stdcall TkWinChildProc(HWND hwnd, UINT message,
				WPARAM wParam, LPARAM lParam);
#endif
#ifndef TkCreateXEventSource_TCL_DECLARED
#define TkCreateXEventSource_TCL_DECLARED
/* 37 */
EXTERN void		TkCreateXEventSource(void);
#endif
#ifndef TkpCmapStressed_TCL_DECLARED
#define TkpCmapStressed_TCL_DECLARED
/* 38 */
EXTERN int		TkpCmapStressed(Tk_Window tkwin, Colormap colormap);
#endif
#ifndef TkpSync_TCL_DECLARED
#define TkpSync_TCL_DECLARED
/* 39 */
EXTERN void		TkpSync(Display *display);
#endif
#ifndef TkUnixContainerId_TCL_DECLARED
#define TkUnixContainerId_TCL_DECLARED
/* 40 */
EXTERN Window		TkUnixContainerId(TkWindow *winPtr);
#endif
#ifndef TkUnixDoOneXEvent_TCL_DECLARED
#define TkUnixDoOneXEvent_TCL_DECLARED
/* 41 */
EXTERN int		TkUnixDoOneXEvent(Tcl_Time *timePtr);
#endif
#ifndef TkUnixSetMenubar_TCL_DECLARED
#define TkUnixSetMenubar_TCL_DECLARED
/* 42 */
EXTERN void		TkUnixSetMenubar(Tk_Window tkwin, Tk_Window menubar);
#endif
#ifndef TkWmCleanup_TCL_DECLARED
#define TkWmCleanup_TCL_DECLARED
/* 43 */
EXTERN void		TkWmCleanup(TkDisplay *dispPtr);
#endif
#ifndef TkSendCleanup_TCL_DECLARED
#define TkSendCleanup_TCL_DECLARED
/* 44 */
EXTERN void		TkSendCleanup(TkDisplay *dispPtr);
#endif
#ifndef TkpTestsendCmd_TCL_DECLARED
#define TkpTestsendCmd_TCL_DECLARED
/* 45 */
EXTERN int		TkpTestsendCmd(ClientData clientData,
				Tcl_Interp *interp, int argc,
				CONST char **argv);
#endif
#endif /* WIN */
#ifdef MAC_OSX_TK /* AQUA */
#ifndef TkGenerateActivateEvents_TCL_DECLARED
#define TkGenerateActivateEvents_TCL_DECLARED
/* 0 */
EXTERN void		TkGenerateActivateEvents(TkWindow *winPtr,
				int active);
#endif
/* Slot 1 is reserved */
/* Slot 2 is reserved */
#ifndef TkPointerDeadWindow_TCL_DECLARED
#define TkPointerDeadWindow_TCL_DECLARED
/* 3 */
EXTERN void		TkPointerDeadWindow(TkWindow *winPtr);
#endif
#ifndef TkpSetCapture_TCL_DECLARED
#define TkpSetCapture_TCL_DECLARED
/* 4 */
EXTERN void		TkpSetCapture(TkWindow *winPtr);
#endif
#ifndef TkpSetCursor_TCL_DECLARED
#define TkpSetCursor_TCL_DECLARED
/* 5 */
EXTERN void		TkpSetCursor(TkpCursor cursor);
#endif
#ifndef TkpWmSetState_TCL_DECLARED
#define TkpWmSetState_TCL_DECLARED
/* 6 */
EXTERN void		TkpWmSetState(TkWindow *winPtr, int state);
#endif
#ifndef TkAboutDlg_TCL_DECLARED
#define TkAboutDlg_TCL_DECLARED
/* 7 */
EXTERN void		TkAboutDlg(void);
#endif
#ifndef TkMacOSXButtonKeyState_TCL_DECLARED
#define TkMacOSXButtonKeyState_TCL_DECLARED
/* 8 */
EXTERN unsigned int	TkMacOSXButtonKeyState(void);
#endif
#ifndef TkMacOSXClearMenubarActive_TCL_DECLARED
#define TkMacOSXClearMenubarActive_TCL_DECLARED
/* 9 */
EXTERN void		TkMacOSXClearMenubarActive(void);
#endif
#ifndef TkMacOSXDispatchMenuEvent_TCL_DECLARED
#define TkMacOSXDispatchMenuEvent_TCL_DECLARED
/* 10 */
EXTERN int		TkMacOSXDispatchMenuEvent(int menuID, int index);
#endif
#ifndef TkMacOSXInstallCursor_TCL_DECLARED
#define TkMacOSXInstallCursor_TCL_DECLARED
/* 11 */
EXTERN void		TkMacOSXInstallCursor(int resizeOverride);
#endif
#ifndef TkMacOSXHandleTearoffMenu_TCL_DECLARED
#define TkMacOSXHandleTearoffMenu_TCL_DECLARED
/* 12 */
EXTERN void		TkMacOSXHandleTearoffMenu(void);
#endif
/* Slot 13 is reserved */
#ifndef TkMacOSXDoHLEvent_TCL_DECLARED
#define TkMacOSXDoHLEvent_TCL_DECLARED
/* 14 */
EXTERN int		TkMacOSXDoHLEvent(VOID *theEvent);
#endif
/* Slot 15 is reserved */
#ifndef TkMacOSXGetXWindow_TCL_DECLARED
#define TkMacOSXGetXWindow_TCL_DECLARED
/* 16 */
EXTERN Window		TkMacOSXGetXWindow(VOID *macWinPtr);
#endif
#ifndef TkMacOSXGrowToplevel_TCL_DECLARED
#define TkMacOSXGrowToplevel_TCL_DECLARED
/* 17 */
EXTERN int		TkMacOSXGrowToplevel(VOID *whichWindow, XPoint start);
#endif
#ifndef TkMacOSXHandleMenuSelect_TCL_DECLARED
#define TkMacOSXHandleMenuSelect_TCL_DECLARED
/* 18 */
EXTERN void		TkMacOSXHandleMenuSelect(short theMenu,
				unsigned short theItem, int optionKeyPressed);
#endif
/* Slot 19 is reserved */
/* Slot 20 is reserved */
#ifndef TkMacOSXInvalidateWindow_TCL_DECLARED
#define TkMacOSXInvalidateWindow_TCL_DECLARED
/* 21 */
EXTERN void		TkMacOSXInvalidateWindow(MacDrawable *macWin,
				int flag);
#endif
#ifndef TkMacOSXIsCharacterMissing_TCL_DECLARED
#define TkMacOSXIsCharacterMissing_TCL_DECLARED
/* 22 */
EXTERN int		TkMacOSXIsCharacterMissing(Tk_Font tkfont,
				unsigned int searchChar);
#endif
#ifndef TkMacOSXMakeRealWindowExist_TCL_DECLARED
#define TkMacOSXMakeRealWindowExist_TCL_DECLARED
/* 23 */
EXTERN void		TkMacOSXMakeRealWindowExist(TkWindow *winPtr);
#endif
#ifndef TkMacOSXMakeStippleMap_TCL_DECLARED
#define TkMacOSXMakeStippleMap_TCL_DECLARED
/* 24 */
EXTERN VOID *		TkMacOSXMakeStippleMap(Drawable d1, Drawable d2);
#endif
#ifndef TkMacOSXMenuClick_TCL_DECLARED
#define TkMacOSXMenuClick_TCL_DECLARED
/* 25 */
EXTERN void		TkMacOSXMenuClick(void);
#endif
#ifndef TkMacOSXRegisterOffScreenWindow_TCL_DECLARED
#define TkMacOSXRegisterOffScreenWindow_TCL_DECLARED
/* 26 */
EXTERN void		TkMacOSXRegisterOffScreenWindow(Window window,
				VOID *portPtr);
#endif
#ifndef TkMacOSXResizable_TCL_DECLARED
#define TkMacOSXResizable_TCL_DECLARED
/* 27 */
EXTERN int		TkMacOSXResizable(TkWindow *winPtr);
#endif
#ifndef TkMacOSXSetHelpMenuItemCount_TCL_DECLARED
#define TkMacOSXSetHelpMenuItemCount_TCL_DECLARED
/* 28 */
EXTERN void		TkMacOSXSetHelpMenuItemCount(void);
#endif
#ifndef TkMacOSXSetScrollbarGrow_TCL_DECLARED
#define TkMacOSXSetScrollbarGrow_TCL_DECLARED
/* 29 */
EXTERN void		TkMacOSXSetScrollbarGrow(TkWindow *winPtr, int flag);
#endif
#ifndef TkMacOSXSetUpClippingRgn_TCL_DECLARED
#define TkMacOSXSetUpClippingRgn_TCL_DECLARED
/* 30 */
EXTERN void		TkMacOSXSetUpClippingRgn(Drawable drawable);
#endif
#ifndef TkMacOSXSetUpGraphicsPort_TCL_DECLARED
#define TkMacOSXSetUpGraphicsPort_TCL_DECLARED
/* 31 */
EXTERN void		TkMacOSXSetUpGraphicsPort(GC gc, VOID *destPort);
#endif
#ifndef TkMacOSXUpdateClipRgn_TCL_DECLARED
#define TkMacOSXUpdateClipRgn_TCL_DECLARED
/* 32 */
EXTERN void		TkMacOSXUpdateClipRgn(TkWindow *winPtr);
#endif
#ifndef TkMacOSXUnregisterMacWindow_TCL_DECLARED
#define TkMacOSXUnregisterMacWindow_TCL_DECLARED
/* 33 */
EXTERN void		TkMacOSXUnregisterMacWindow(VOID *portPtr);
#endif
#ifndef TkMacOSXUseMenuID_TCL_DECLARED
#define TkMacOSXUseMenuID_TCL_DECLARED
/* 34 */
EXTERN int		TkMacOSXUseMenuID(short macID);
#endif
#ifndef TkMacOSXVisableClipRgn_TCL_DECLARED
#define TkMacOSXVisableClipRgn_TCL_DECLARED
/* 35 */
EXTERN TkRegion		TkMacOSXVisableClipRgn(TkWindow *winPtr);
#endif
#ifndef TkMacOSXWinBounds_TCL_DECLARED
#define TkMacOSXWinBounds_TCL_DECLARED
/* 36 */
EXTERN void		TkMacOSXWinBounds(TkWindow *winPtr, VOID *geometry);
#endif
#ifndef TkMacOSXWindowOffset_TCL_DECLARED
#define TkMacOSXWindowOffset_TCL_DECLARED
/* 37 */
EXTERN void		TkMacOSXWindowOffset(VOID *wRef, int *xOffset,
				int *yOffset);
#endif
#ifndef TkSetMacColor_TCL_DECLARED
#define TkSetMacColor_TCL_DECLARED
/* 38 */
EXTERN int		TkSetMacColor(unsigned long pixel, VOID *macColor);
#endif
#ifndef TkSetWMName_TCL_DECLARED
#define TkSetWMName_TCL_DECLARED
/* 39 */
EXTERN void		TkSetWMName(TkWindow *winPtr, Tk_Uid titleUid);
#endif
#ifndef TkSuspendClipboard_TCL_DECLARED
#define TkSuspendClipboard_TCL_DECLARED
/* 40 */
EXTERN void		TkSuspendClipboard(void);
#endif
#ifndef TkMacOSXZoomToplevel_TCL_DECLARED
#define TkMacOSXZoomToplevel_TCL_DECLARED
/* 41 */
EXTERN int		TkMacOSXZoomToplevel(VOID *whichWindow,
				short zoomPart);
#endif
#ifndef Tk_TopCoordsToWindow_TCL_DECLARED
#define Tk_TopCoordsToWindow_TCL_DECLARED
/* 42 */
EXTERN Tk_Window	Tk_TopCoordsToWindow(Tk_Window tkwin, int rootX,
				int rootY, int *newX, int *newY);
#endif
#ifndef TkMacOSXContainerId_TCL_DECLARED
#define TkMacOSXContainerId_TCL_DECLARED
/* 43 */
EXTERN MacDrawable *	TkMacOSXContainerId(TkWindow *winPtr);
#endif
#ifndef TkMacOSXGetHostToplevel_TCL_DECLARED
#define TkMacOSXGetHostToplevel_TCL_DECLARED
/* 44 */
EXTERN MacDrawable *	TkMacOSXGetHostToplevel(TkWindow *winPtr);
#endif
#ifndef TkMacOSXPreprocessMenu_TCL_DECLARED
#define TkMacOSXPreprocessMenu_TCL_DECLARED
/* 45 */
EXTERN void		TkMacOSXPreprocessMenu(void);
#endif
#ifndef TkpIsWindowFloating_TCL_DECLARED
#define TkpIsWindowFloating_TCL_DECLARED
/* 46 */
EXTERN int		TkpIsWindowFloating(VOID *window);
#endif
#ifndef TkMacOSXGetCapture_TCL_DECLARED
#define TkMacOSXGetCapture_TCL_DECLARED
/* 47 */
EXTERN Tk_Window	TkMacOSXGetCapture(void);
#endif
/* Slot 48 is reserved */
#ifndef TkGetTransientMaster_TCL_DECLARED
#define TkGetTransientMaster_TCL_DECLARED
/* 49 */
EXTERN Window		TkGetTransientMaster(TkWindow *winPtr);
#endif
#ifndef TkGenerateButtonEvent_TCL_DECLARED
#define TkGenerateButtonEvent_TCL_DECLARED
/* 50 */
EXTERN int		TkGenerateButtonEvent(int x, int y, Window window,
				unsigned int state);
#endif
#ifndef TkGenWMDestroyEvent_TCL_DECLARED
#define TkGenWMDestroyEvent_TCL_DECLARED
/* 51 */
EXTERN void		TkGenWMDestroyEvent(Tk_Window tkwin);
#endif
#ifndef TkMacOSXSetDrawingEnabled_TCL_DECLARED
#define TkMacOSXSetDrawingEnabled_TCL_DECLARED
/* 52 */
EXTERN void		TkMacOSXSetDrawingEnabled(TkWindow *winPtr, int flag);
#endif
#ifndef TkpGetMS_TCL_DECLARED
#define TkpGetMS_TCL_DECLARED
/* 53 */
EXTERN unsigned long	TkpGetMS(void);
#endif
#ifndef TkMacOSXDrawable_TCL_DECLARED
#define TkMacOSXDrawable_TCL_DECLARED
/* 54 */
EXTERN VOID *		TkMacOSXDrawable(Drawable drawable);
#endif
#ifndef TkpScanWindowId_TCL_DECLARED
#define TkpScanWindowId_TCL_DECLARED
/* 55 */
EXTERN int		TkpScanWindowId(Tcl_Interp *interp,
				CONST char *string, Window *idPtr);
#endif
#endif /* AQUA */
#if !(defined(__WIN32__) || defined(__CYGWIN__) || defined(MAC_OSX_TK)) /* X11 */
#ifndef TkCreateXEventSource_TCL_DECLARED
#define TkCreateXEventSource_TCL_DECLARED
/* 0 */
EXTERN void		TkCreateXEventSource(void);
#endif
#ifndef TkFreeWindowId_TCL_DECLARED
#define TkFreeWindowId_TCL_DECLARED
/* 1 */
EXTERN void		TkFreeWindowId(TkDisplay *dispPtr, Window w);
#endif
#ifndef TkInitXId_TCL_DECLARED
#define TkInitXId_TCL_DECLARED
/* 2 */
EXTERN void		TkInitXId(TkDisplay *dispPtr);
#endif
#ifndef TkpCmapStressed_TCL_DECLARED
#define TkpCmapStressed_TCL_DECLARED
/* 3 */
EXTERN int		TkpCmapStressed(Tk_Window tkwin, Colormap colormap);
#endif
#ifndef TkpSync_TCL_DECLARED
#define TkpSync_TCL_DECLARED
/* 4 */
EXTERN void		TkpSync(Display *display);
#endif
#ifndef TkUnixContainerId_TCL_DECLARED
#define TkUnixContainerId_TCL_DECLARED
/* 5 */
EXTERN Window		TkUnixContainerId(TkWindow *winPtr);
#endif
#ifndef TkUnixDoOneXEvent_TCL_DECLARED
#define TkUnixDoOneXEvent_TCL_DECLARED
/* 6 */
EXTERN int		TkUnixDoOneXEvent(Tcl_Time *timePtr);
#endif
#ifndef TkUnixSetMenubar_TCL_DECLARED
#define TkUnixSetMenubar_TCL_DECLARED
/* 7 */
EXTERN void		TkUnixSetMenubar(Tk_Window tkwin, Tk_Window menubar);
#endif
#ifndef TkpScanWindowId_TCL_DECLARED
#define TkpScanWindowId_TCL_DECLARED
/* 8 */
EXTERN int		TkpScanWindowId(Tcl_Interp *interp,
				CONST char *string, Window *idPtr);
#endif
#ifndef TkWmCleanup_TCL_DECLARED
#define TkWmCleanup_TCL_DECLARED
/* 9 */
EXTERN void		TkWmCleanup(TkDisplay *dispPtr);
#endif
#ifndef TkSendCleanup_TCL_DECLARED
#define TkSendCleanup_TCL_DECLARED
/* 10 */
EXTERN void		TkSendCleanup(TkDisplay *dispPtr);
#endif
#ifndef TkFreeXId_TCL_DECLARED
#define TkFreeXId_TCL_DECLARED
/* 11 */
EXTERN void		TkFreeXId(TkDisplay *dispPtr);
#endif
#ifndef TkpWmSetState_TCL_DECLARED
#define TkpWmSetState_TCL_DECLARED
/* 12 */
EXTERN int		TkpWmSetState(TkWindow *winPtr, int state);
#endif
#ifndef TkpTestsendCmd_TCL_DECLARED
#define TkpTestsendCmd_TCL_DECLARED
/* 13 */
EXTERN int		TkpTestsendCmd(ClientData clientData,
				Tcl_Interp *interp, int argc,
				CONST char **argv);
#endif
#endif /* X11 */

typedef struct TkIntPlatStubs {
    int magic;
    struct TkIntPlatStubHooks *hooks;

#if defined(__WIN32__) || defined(__CYGWIN__) /* WIN */
    char * (*tkAlignImageData) (XImage *image, int alignment, int bitOrder); /* 0 */
    VOID *reserved1;
    void (*tkGenerateActivateEvents) (TkWindow *winPtr, int active); /* 2 */
    unsigned long (*tkpGetMS) (void); /* 3 */
    void (*tkPointerDeadWindow) (TkWindow *winPtr); /* 4 */
    void (*tkpPrintWindowId) (char *buf, Window window); /* 5 */
    int (*tkpScanWindowId) (Tcl_Interp *interp, CONST char *string, Window *idPtr); /* 6 */
    void (*tkpSetCapture) (TkWindow *winPtr); /* 7 */
    void (*tkpSetCursor) (TkpCursor cursor); /* 8 */
    int (*tkpWmSetState) (TkWindow *winPtr, int state); /* 9 */
    void (*tkSetPixmapColormap) (Pixmap pixmap, Colormap colormap); /* 10 */
    void (*tkWinCancelMouseTimer) (void); /* 11 */
    void (*tkWinClipboardRender) (TkDisplay *dispPtr, UINT format); /* 12 */
    LRESULT (*tkWinEmbeddedEventProc) (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam); /* 13 */
    void (*tkWinFillRect) (HDC dc, int x, int y, int width, int height, int pixel); /* 14 */
    COLORREF (*tkWinGetBorderPixels) (Tk_Window tkwin, Tk_3DBorder border, int which); /* 15 */
    HDC (*tkWinGetDrawableDC) (Display *display, Drawable d, TkWinDCState *state); /* 16 */
    int (*tkWinGetModifierState) (void); /* 17 */
    HPALETTE (*tkWinGetSystemPalette) (void); /* 18 */
    HWND (*tkWinGetWrapperWindow) (Tk_Window tkwin); /* 19 */
    int (*tkWinHandleMenuEvent) (HWND *phwnd, UINT *pMessage, WPARAM *pwParam, LPARAM *plParam, LRESULT *plResult); /* 20 */
    int (*tkWinIndexOfColor) (XColor *colorPtr); /* 21 */
    void (*tkWinReleaseDrawableDC) (Drawable d, HDC hdc, TkWinDCState *state); /* 22 */
    LRESULT (*tkWinResendEvent) (WNDPROC wndproc, HWND hwnd, XEvent *eventPtr); /* 23 */
    HPALETTE (*tkWinSelectPalette) (HDC dc, Colormap colormap); /* 24 */
    void (*tkWinSetMenu) (Tk_Window tkwin, HMENU hMenu); /* 25 */
    void (*tkWinSetWindowPos) (HWND hwnd, HWND siblingHwnd, int pos); /* 26 */
    void (*tkWinWmCleanup) (HINSTANCE hInstance); /* 27 */
    void (*tkWinXCleanup) (ClientData clientData); /* 28 */
    void (*tkWinXInit) (HINSTANCE hInstance); /* 29 */
    void (*tkWinSetForegroundWindow) (TkWindow *winPtr); /* 30 */
    void (*tkWinDialogDebug) (int debug); /* 31 */
    Tcl_Obj * (*tkWinGetMenuSystemDefault) (Tk_Window tkwin, CONST char *dbName, CONST char *className); /* 32 */
    int (*tkWinGetPlatformId) (void); /* 33 */
    void (*tkWinSetHINSTANCE) (HINSTANCE hInstance); /* 34 */
    int (*tkWinGetPlatformTheme) (void); /* 35 */
    LRESULT (__stdcall *tkWinChildProc) (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam); /* 36 */
    void (*tkCreateXEventSource) (void); /* 37 */
    int (*tkpCmapStressed) (Tk_Window tkwin, Colormap colormap); /* 38 */
    void (*tkpSync) (Display *display); /* 39 */
    Window (*tkUnixContainerId) (TkWindow *winPtr); /* 40 */
    int (*tkUnixDoOneXEvent) (Tcl_Time *timePtr); /* 41 */
    void (*tkUnixSetMenubar) (Tk_Window tkwin, Tk_Window menubar); /* 42 */
    void (*tkWmCleanup) (TkDisplay *dispPtr); /* 43 */
    void (*tkSendCleanup) (TkDisplay *dispPtr); /* 44 */
    int (*tkpTestsendCmd) (ClientData clientData, Tcl_Interp *interp, int argc, CONST char **argv); /* 45 */
#endif /* WIN */
#ifdef MAC_OSX_TK /* AQUA */
    void (*tkGenerateActivateEvents) (TkWindow *winPtr, int active); /* 0 */
    VOID *reserved1;
    VOID *reserved2;
    void (*tkPointerDeadWindow) (TkWindow *winPtr); /* 3 */
    void (*tkpSetCapture) (TkWindow *winPtr); /* 4 */
    void (*tkpSetCursor) (TkpCursor cursor); /* 5 */
    void (*tkpWmSetState) (TkWindow *winPtr, int state); /* 6 */
    void (*tkAboutDlg) (void); /* 7 */
    unsigned int (*tkMacOSXButtonKeyState) (void); /* 8 */
    void (*tkMacOSXClearMenubarActive) (void); /* 9 */
    int (*tkMacOSXDispatchMenuEvent) (int menuID, int index); /* 10 */
    void (*tkMacOSXInstallCursor) (int resizeOverride); /* 11 */
    void (*tkMacOSXHandleTearoffMenu) (void); /* 12 */
    VOID *reserved13;
    int (*tkMacOSXDoHLEvent) (VOID *theEvent); /* 14 */
    VOID *reserved15;
    Window (*tkMacOSXGetXWindow) (VOID *macWinPtr); /* 16 */
    int (*tkMacOSXGrowToplevel) (VOID *whichWindow, XPoint start); /* 17 */
    void (*tkMacOSXHandleMenuSelect) (short theMenu, unsigned short theItem, int optionKeyPressed); /* 18 */
    VOID *reserved19;
    VOID *reserved20;
    void (*tkMacOSXInvalidateWindow) (MacDrawable *macWin, int flag); /* 21 */
    int (*tkMacOSXIsCharacterMissing) (Tk_Font tkfont, unsigned int searchChar); /* 22 */
    void (*tkMacOSXMakeRealWindowExist) (TkWindow *winPtr); /* 23 */
    VOID * (*tkMacOSXMakeStippleMap) (Drawable d1, Drawable d2); /* 24 */
    void (*tkMacOSXMenuClick) (void); /* 25 */
    void (*tkMacOSXRegisterOffScreenWindow) (Window window, VOID *portPtr); /* 26 */
    int (*tkMacOSXResizable) (TkWindow *winPtr); /* 27 */
    void (*tkMacOSXSetHelpMenuItemCount) (void); /* 28 */
    void (*tkMacOSXSetScrollbarGrow) (TkWindow *winPtr, int flag); /* 29 */
    void (*tkMacOSXSetUpClippingRgn) (Drawable drawable); /* 30 */
    void (*tkMacOSXSetUpGraphicsPort) (GC gc, VOID *destPort); /* 31 */
    void (*tkMacOSXUpdateClipRgn) (TkWindow *winPtr); /* 32 */
    void (*tkMacOSXUnregisterMacWindow) (VOID *portPtr); /* 33 */
    int (*tkMacOSXUseMenuID) (short macID); /* 34 */
    TkRegion (*tkMacOSXVisableClipRgn) (TkWindow *winPtr); /* 35 */
    void (*tkMacOSXWinBounds) (TkWindow *winPtr, VOID *geometry); /* 36 */
    void (*tkMacOSXWindowOffset) (VOID *wRef, int *xOffset, int *yOffset); /* 37 */
    int (*tkSetMacColor) (unsigned long pixel, VOID *macColor); /* 38 */
    void (*tkSetWMName) (TkWindow *winPtr, Tk_Uid titleUid); /* 39 */
    void (*tkSuspendClipboard) (void); /* 40 */
    int (*tkMacOSXZoomToplevel) (VOID *whichWindow, short zoomPart); /* 41 */
    Tk_Window (*tk_TopCoordsToWindow) (Tk_Window tkwin, int rootX, int rootY, int *newX, int *newY); /* 42 */
    MacDrawable * (*tkMacOSXContainerId) (TkWindow *winPtr); /* 43 */
    MacDrawable * (*tkMacOSXGetHostToplevel) (TkWindow *winPtr); /* 44 */
    void (*tkMacOSXPreprocessMenu) (void); /* 45 */
    int (*tkpIsWindowFloating) (VOID *window); /* 46 */
    Tk_Window (*tkMacOSXGetCapture) (void); /* 47 */
    VOID *reserved48;
    Window (*tkGetTransientMaster) (TkWindow *winPtr); /* 49 */
    int (*tkGenerateButtonEvent) (int x, int y, Window window, unsigned int state); /* 50 */
    void (*tkGenWMDestroyEvent) (Tk_Window tkwin); /* 51 */
    void (*tkMacOSXSetDrawingEnabled) (TkWindow *winPtr, int flag); /* 52 */
    unsigned long (*tkpGetMS) (void); /* 53 */
    VOID * (*tkMacOSXDrawable) (Drawable drawable); /* 54 */
    int (*tkpScanWindowId) (Tcl_Interp *interp, CONST char *string, Window *idPtr); /* 55 */
#endif /* AQUA */
#if !(defined(__WIN32__) || defined(__CYGWIN__) || defined(MAC_OSX_TK)) /* X11 */
    void (*tkCreateXEventSource) (void); /* 0 */
    void (*tkFreeWindowId) (TkDisplay *dispPtr, Window w); /* 1 */
    void (*tkInitXId) (TkDisplay *dispPtr); /* 2 */
    int (*tkpCmapStressed) (Tk_Window tkwin, Colormap colormap); /* 3 */
    void (*tkpSync) (Display *display); /* 4 */
    Window (*tkUnixContainerId) (TkWindow *winPtr); /* 5 */
    int (*tkUnixDoOneXEvent) (Tcl_Time *timePtr); /* 6 */
    void (*tkUnixSetMenubar) (Tk_Window tkwin, Tk_Window menubar); /* 7 */
    int (*tkpScanWindowId) (Tcl_Interp *interp, CONST char *string, Window *idPtr); /* 8 */
    void (*tkWmCleanup) (TkDisplay *dispPtr); /* 9 */
    void (*tkSendCleanup) (TkDisplay *dispPtr); /* 10 */
    void (*tkFreeXId) (TkDisplay *dispPtr); /* 11 */
    int (*tkpWmSetState) (TkWindow *winPtr, int state); /* 12 */
    int (*tkpTestsendCmd) (ClientData clientData, Tcl_Interp *interp, int argc, CONST char **argv); /* 13 */
#endif /* X11 */
} TkIntPlatStubs;

extern TkIntPlatStubs *tkIntPlatStubsPtr;

#ifdef __cplusplus
}
#endif

#if defined(USE_TK_STUBS) && !defined(USE_TK_STUB_PROCS)

/*
 * Inline function declarations:
 */

#if defined(__WIN32__) || defined(__CYGWIN__) /* WIN */
#ifndef TkAlignImageData
#define TkAlignImageData \
	(tkIntPlatStubsPtr->tkAlignImageData) /* 0 */
#endif
/* Slot 1 is reserved */
#ifndef TkGenerateActivateEvents
#define TkGenerateActivateEvents \
	(tkIntPlatStubsPtr->tkGenerateActivateEvents) /* 2 */
#endif
#ifndef TkpGetMS
#define TkpGetMS \
	(tkIntPlatStubsPtr->tkpGetMS) /* 3 */
#endif
#ifndef TkPointerDeadWindow
#define TkPointerDeadWindow \
	(tkIntPlatStubsPtr->tkPointerDeadWindow) /* 4 */
#endif
#ifndef TkpPrintWindowId
#define TkpPrintWindowId \
	(tkIntPlatStubsPtr->tkpPrintWindowId) /* 5 */
#endif
#ifndef TkpScanWindowId
#define TkpScanWindowId \
	(tkIntPlatStubsPtr->tkpScanWindowId) /* 6 */
#endif
#ifndef TkpSetCapture
#define TkpSetCapture \
	(tkIntPlatStubsPtr->tkpSetCapture) /* 7 */
#endif
#ifndef TkpSetCursor
#define TkpSetCursor \
	(tkIntPlatStubsPtr->tkpSetCursor) /* 8 */
#endif
#ifndef TkpWmSetState
#define TkpWmSetState \
	(tkIntPlatStubsPtr->tkpWmSetState) /* 9 */
#endif
#ifndef TkSetPixmapColormap
#define TkSetPixmapColormap \
	(tkIntPlatStubsPtr->tkSetPixmapColormap) /* 10 */
#endif
#ifndef TkWinCancelMouseTimer
#define TkWinCancelMouseTimer \
	(tkIntPlatStubsPtr->tkWinCancelMouseTimer) /* 11 */
#endif
#ifndef TkWinClipboardRender
#define TkWinClipboardRender \
	(tkIntPlatStubsPtr->tkWinClipboardRender) /* 12 */
#endif
#ifndef TkWinEmbeddedEventProc
#define TkWinEmbeddedEventProc \
	(tkIntPlatStubsPtr->tkWinEmbeddedEventProc) /* 13 */
#endif
#ifndef TkWinFillRect
#define TkWinFillRect \
	(tkIntPlatStubsPtr->tkWinFillRect) /* 14 */
#endif
#ifndef TkWinGetBorderPixels
#define TkWinGetBorderPixels \
	(tkIntPlatStubsPtr->tkWinGetBorderPixels) /* 15 */
#endif
#ifndef TkWinGetDrawableDC
#define TkWinGetDrawableDC \
	(tkIntPlatStubsPtr->tkWinGetDrawableDC) /* 16 */
#endif
#ifndef TkWinGetModifierState
#define TkWinGetModifierState \
	(tkIntPlatStubsPtr->tkWinGetModifierState) /* 17 */
#endif
#ifndef TkWinGetSystemPalette
#define TkWinGetSystemPalette \
	(tkIntPlatStubsPtr->tkWinGetSystemPalette) /* 18 */
#endif
#ifndef TkWinGetWrapperWindow
#define TkWinGetWrapperWindow \
	(tkIntPlatStubsPtr->tkWinGetWrapperWindow) /* 19 */
#endif
#ifndef TkWinHandleMenuEvent
#define TkWinHandleMenuEvent \
	(tkIntPlatStubsPtr->tkWinHandleMenuEvent) /* 20 */
#endif
#ifndef TkWinIndexOfColor
#define TkWinIndexOfColor \
	(tkIntPlatStubsPtr->tkWinIndexOfColor) /* 21 */
#endif
#ifndef TkWinReleaseDrawableDC
#define TkWinReleaseDrawableDC \
	(tkIntPlatStubsPtr->tkWinReleaseDrawableDC) /* 22 */
#endif
#ifndef TkWinResendEvent
#define TkWinResendEvent \
	(tkIntPlatStubsPtr->tkWinResendEvent) /* 23 */
#endif
#ifndef TkWinSelectPalette
#define TkWinSelectPalette \
	(tkIntPlatStubsPtr->tkWinSelectPalette) /* 24 */
#endif
#ifndef TkWinSetMenu
#define TkWinSetMenu \
	(tkIntPlatStubsPtr->tkWinSetMenu) /* 25 */
#endif
#ifndef TkWinSetWindowPos
#define TkWinSetWindowPos \
	(tkIntPlatStubsPtr->tkWinSetWindowPos) /* 26 */
#endif
#ifndef TkWinWmCleanup
#define TkWinWmCleanup \
	(tkIntPlatStubsPtr->tkWinWmCleanup) /* 27 */
#endif
#ifndef TkWinXCleanup
#define TkWinXCleanup \
	(tkIntPlatStubsPtr->tkWinXCleanup) /* 28 */
#endif
#ifndef TkWinXInit
#define TkWinXInit \
	(tkIntPlatStubsPtr->tkWinXInit) /* 29 */
#endif
#ifndef TkWinSetForegroundWindow
#define TkWinSetForegroundWindow \
	(tkIntPlatStubsPtr->tkWinSetForegroundWindow) /* 30 */
#endif
#ifndef TkWinDialogDebug
#define TkWinDialogDebug \
	(tkIntPlatStubsPtr->tkWinDialogDebug) /* 31 */
#endif
#ifndef TkWinGetMenuSystemDefault
#define TkWinGetMenuSystemDefault \
	(tkIntPlatStubsPtr->tkWinGetMenuSystemDefault) /* 32 */
#endif
#ifndef TkWinGetPlatformId
#define TkWinGetPlatformId \
	(tkIntPlatStubsPtr->tkWinGetPlatformId) /* 33 */
#endif
#ifndef TkWinSetHINSTANCE
#define TkWinSetHINSTANCE \
	(tkIntPlatStubsPtr->tkWinSetHINSTANCE) /* 34 */
#endif
#ifndef TkWinGetPlatformTheme
#define TkWinGetPlatformTheme \
	(tkIntPlatStubsPtr->tkWinGetPlatformTheme) /* 35 */
#endif
#ifndef TkWinChildProc
#define TkWinChildProc \
	(tkIntPlatStubsPtr->tkWinChildProc) /* 36 */
#endif
#ifndef TkCreateXEventSource
#define TkCreateXEventSource \
	(tkIntPlatStubsPtr->tkCreateXEventSource) /* 37 */
#endif
#ifndef TkpCmapStressed
#define TkpCmapStressed \
	(tkIntPlatStubsPtr->tkpCmapStressed) /* 38 */
#endif
#ifndef TkpSync
#define TkpSync \
	(tkIntPlatStubsPtr->tkpSync) /* 39 */
#endif
#ifndef TkUnixContainerId
#define TkUnixContainerId \
	(tkIntPlatStubsPtr->tkUnixContainerId) /* 40 */
#endif
#ifndef TkUnixDoOneXEvent
#define TkUnixDoOneXEvent \
	(tkIntPlatStubsPtr->tkUnixDoOneXEvent) /* 41 */
#endif
#ifndef TkUnixSetMenubar
#define TkUnixSetMenubar \
	(tkIntPlatStubsPtr->tkUnixSetMenubar) /* 42 */
#endif
#ifndef TkWmCleanup
#define TkWmCleanup \
	(tkIntPlatStubsPtr->tkWmCleanup) /* 43 */
#endif
#ifndef TkSendCleanup
#define TkSendCleanup \
	(tkIntPlatStubsPtr->tkSendCleanup) /* 44 */
#endif
#ifndef TkpTestsendCmd
#define TkpTestsendCmd \
	(tkIntPlatStubsPtr->tkpTestsendCmd) /* 45 */
#endif
#endif /* WIN */
#ifdef MAC_OSX_TK /* AQUA */
#ifndef TkGenerateActivateEvents
#define TkGenerateActivateEvents \
	(tkIntPlatStubsPtr->tkGenerateActivateEvents) /* 0 */
#endif
/* Slot 1 is reserved */
/* Slot 2 is reserved */
#ifndef TkPointerDeadWindow
#define TkPointerDeadWindow \
	(tkIntPlatStubsPtr->tkPointerDeadWindow) /* 3 */
#endif
#ifndef TkpSetCapture
#define TkpSetCapture \
	(tkIntPlatStubsPtr->tkpSetCapture) /* 4 */
#endif
#ifndef TkpSetCursor
#define TkpSetCursor \
	(tkIntPlatStubsPtr->tkpSetCursor) /* 5 */
#endif
#ifndef TkpWmSetState
#define TkpWmSetState \
	(tkIntPlatStubsPtr->tkpWmSetState) /* 6 */
#endif
#ifndef TkAboutDlg
#define TkAboutDlg \
	(tkIntPlatStubsPtr->tkAboutDlg) /* 7 */
#endif
#ifndef TkMacOSXButtonKeyState
#define TkMacOSXButtonKeyState \
	(tkIntPlatStubsPtr->tkMacOSXButtonKeyState) /* 8 */
#endif
#ifndef TkMacOSXClearMenubarActive
#define TkMacOSXClearMenubarActive \
	(tkIntPlatStubsPtr->tkMacOSXClearMenubarActive) /* 9 */
#endif
#ifndef TkMacOSXDispatchMenuEvent
#define TkMacOSXDispatchMenuEvent \
	(tkIntPlatStubsPtr->tkMacOSXDispatchMenuEvent) /* 10 */
#endif
#ifndef TkMacOSXInstallCursor
#define TkMacOSXInstallCursor \
	(tkIntPlatStubsPtr->tkMacOSXInstallCursor) /* 11 */
#endif
#ifndef TkMacOSXHandleTearoffMenu
#define TkMacOSXHandleTearoffMenu \
	(tkIntPlatStubsPtr->tkMacOSXHandleTearoffMenu) /* 12 */
#endif
/* Slot 13 is reserved */
#ifndef TkMacOSXDoHLEvent
#define TkMacOSXDoHLEvent \
	(tkIntPlatStubsPtr->tkMacOSXDoHLEvent) /* 14 */
#endif
/* Slot 15 is reserved */
#ifndef TkMacOSXGetXWindow
#define TkMacOSXGetXWindow \
	(tkIntPlatStubsPtr->tkMacOSXGetXWindow) /* 16 */
#endif
#ifndef TkMacOSXGrowToplevel
#define TkMacOSXGrowToplevel \
	(tkIntPlatStubsPtr->tkMacOSXGrowToplevel) /* 17 */
#endif
#ifndef TkMacOSXHandleMenuSelect
#define TkMacOSXHandleMenuSelect \
	(tkIntPlatStubsPtr->tkMacOSXHandleMenuSelect) /* 18 */
#endif
/* Slot 19 is reserved */
/* Slot 20 is reserved */
#ifndef TkMacOSXInvalidateWindow
#define TkMacOSXInvalidateWindow \
	(tkIntPlatStubsPtr->tkMacOSXInvalidateWindow) /* 21 */
#endif
#ifndef TkMacOSXIsCharacterMissing
#define TkMacOSXIsCharacterMissing \
	(tkIntPlatStubsPtr->tkMacOSXIsCharacterMissing) /* 22 */
#endif
#ifndef TkMacOSXMakeRealWindowExist
#define TkMacOSXMakeRealWindowExist \
	(tkIntPlatStubsPtr->tkMacOSXMakeRealWindowExist) /* 23 */
#endif
#ifndef TkMacOSXMakeStippleMap
#define TkMacOSXMakeStippleMap \
	(tkIntPlatStubsPtr->tkMacOSXMakeStippleMap) /* 24 */
#endif
#ifndef TkMacOSXMenuClick
#define TkMacOSXMenuClick \
	(tkIntPlatStubsPtr->tkMacOSXMenuClick) /* 25 */
#endif
#ifndef TkMacOSXRegisterOffScreenWindow
#define TkMacOSXRegisterOffScreenWindow \
	(tkIntPlatStubsPtr->tkMacOSXRegisterOffScreenWindow) /* 26 */
#endif
#ifndef TkMacOSXResizable
#define TkMacOSXResizable \
	(tkIntPlatStubsPtr->tkMacOSXResizable) /* 27 */
#endif
#ifndef TkMacOSXSetHelpMenuItemCount
#define TkMacOSXSetHelpMenuItemCount \
	(tkIntPlatStubsPtr->tkMacOSXSetHelpMenuItemCount) /* 28 */
#endif
#ifndef TkMacOSXSetScrollbarGrow
#define TkMacOSXSetScrollbarGrow \
	(tkIntPlatStubsPtr->tkMacOSXSetScrollbarGrow) /* 29 */
#endif
#ifndef TkMacOSXSetUpClippingRgn
#define TkMacOSXSetUpClippingRgn \
	(tkIntPlatStubsPtr->tkMacOSXSetUpClippingRgn) /* 30 */
#endif
#ifndef TkMacOSXSetUpGraphicsPort
#define TkMacOSXSetUpGraphicsPort \
	(tkIntPlatStubsPtr->tkMacOSXSetUpGraphicsPort) /* 31 */
#endif
#ifndef TkMacOSXUpdateClipRgn
#define TkMacOSXUpdateClipRgn \
	(tkIntPlatStubsPtr->tkMacOSXUpdateClipRgn) /* 32 */
#endif
#ifndef TkMacOSXUnregisterMacWindow
#define TkMacOSXUnregisterMacWindow \
	(tkIntPlatStubsPtr->tkMacOSXUnregisterMacWindow) /* 33 */
#endif
#ifndef TkMacOSXUseMenuID
#define TkMacOSXUseMenuID \
	(tkIntPlatStubsPtr->tkMacOSXUseMenuID) /* 34 */
#endif
#ifndef TkMacOSXVisableClipRgn
#define TkMacOSXVisableClipRgn \
	(tkIntPlatStubsPtr->tkMacOSXVisableClipRgn) /* 35 */
#endif
#ifndef TkMacOSXWinBounds
#define TkMacOSXWinBounds \
	(tkIntPlatStubsPtr->tkMacOSXWinBounds) /* 36 */
#endif
#ifndef TkMacOSXWindowOffset
#define TkMacOSXWindowOffset \
	(tkIntPlatStubsPtr->tkMacOSXWindowOffset) /* 37 */
#endif
#ifndef TkSetMacColor
#define TkSetMacColor \
	(tkIntPlatStubsPtr->tkSetMacColor) /* 38 */
#endif
#ifndef TkSetWMName
#define TkSetWMName \
	(tkIntPlatStubsPtr->tkSetWMName) /* 39 */
#endif
#ifndef TkSuspendClipboard
#define TkSuspendClipboard \
	(tkIntPlatStubsPtr->tkSuspendClipboard) /* 40 */
#endif
#ifndef TkMacOSXZoomToplevel
#define TkMacOSXZoomToplevel \
	(tkIntPlatStubsPtr->tkMacOSXZoomToplevel) /* 41 */
#endif
#ifndef Tk_TopCoordsToWindow
#define Tk_TopCoordsToWindow \
	(tkIntPlatStubsPtr->tk_TopCoordsToWindow) /* 42 */
#endif
#ifndef TkMacOSXContainerId
#define TkMacOSXContainerId \
	(tkIntPlatStubsPtr->tkMacOSXContainerId) /* 43 */
#endif
#ifndef TkMacOSXGetHostToplevel
#define TkMacOSXGetHostToplevel \
	(tkIntPlatStubsPtr->tkMacOSXGetHostToplevel) /* 44 */
#endif
#ifndef TkMacOSXPreprocessMenu
#define TkMacOSXPreprocessMenu \
	(tkIntPlatStubsPtr->tkMacOSXPreprocessMenu) /* 45 */
#endif
#ifndef TkpIsWindowFloating
#define TkpIsWindowFloating \
	(tkIntPlatStubsPtr->tkpIsWindowFloating) /* 46 */
#endif
#ifndef TkMacOSXGetCapture
#define TkMacOSXGetCapture \
	(tkIntPlatStubsPtr->tkMacOSXGetCapture) /* 47 */
#endif
/* Slot 48 is reserved */
#ifndef TkGetTransientMaster
#define TkGetTransientMaster \
	(tkIntPlatStubsPtr->tkGetTransientMaster) /* 49 */
#endif
#ifndef TkGenerateButtonEvent
#define TkGenerateButtonEvent \
	(tkIntPlatStubsPtr->tkGenerateButtonEvent) /* 50 */
#endif
#ifndef TkGenWMDestroyEvent
#define TkGenWMDestroyEvent \
	(tkIntPlatStubsPtr->tkGenWMDestroyEvent) /* 51 */
#endif
#ifndef TkMacOSXSetDrawingEnabled
#define TkMacOSXSetDrawingEnabled \
	(tkIntPlatStubsPtr->tkMacOSXSetDrawingEnabled) /* 52 */
#endif
#ifndef TkpGetMS
#define TkpGetMS \
	(tkIntPlatStubsPtr->tkpGetMS) /* 53 */
#endif
#ifndef TkMacOSXDrawable
#define TkMacOSXDrawable \
	(tkIntPlatStubsPtr->tkMacOSXDrawable) /* 54 */
#endif
#ifndef TkpScanWindowId
#define TkpScanWindowId \
	(tkIntPlatStubsPtr->tkpScanWindowId) /* 55 */
#endif
#endif /* AQUA */
#if !(defined(__WIN32__) || defined(__CYGWIN__) || defined(MAC_OSX_TK)) /* X11 */
#ifndef TkCreateXEventSource
#define TkCreateXEventSource \
	(tkIntPlatStubsPtr->tkCreateXEventSource) /* 0 */
#endif
#ifndef TkFreeWindowId
#define TkFreeWindowId \
	(tkIntPlatStubsPtr->tkFreeWindowId) /* 1 */
#endif
#ifndef TkInitXId
#define TkInitXId \
	(tkIntPlatStubsPtr->tkInitXId) /* 2 */
#endif
#ifndef TkpCmapStressed
#define TkpCmapStressed \
	(tkIntPlatStubsPtr->tkpCmapStressed) /* 3 */
#endif
#ifndef TkpSync
#define TkpSync \
	(tkIntPlatStubsPtr->tkpSync) /* 4 */
#endif
#ifndef TkUnixContainerId
#define TkUnixContainerId \
	(tkIntPlatStubsPtr->tkUnixContainerId) /* 5 */
#endif
#ifndef TkUnixDoOneXEvent
#define TkUnixDoOneXEvent \
	(tkIntPlatStubsPtr->tkUnixDoOneXEvent) /* 6 */
#endif
#ifndef TkUnixSetMenubar
#define TkUnixSetMenubar \
	(tkIntPlatStubsPtr->tkUnixSetMenubar) /* 7 */
#endif
#ifndef TkpScanWindowId
#define TkpScanWindowId \
	(tkIntPlatStubsPtr->tkpScanWindowId) /* 8 */
#endif
#ifndef TkWmCleanup
#define TkWmCleanup \
	(tkIntPlatStubsPtr->tkWmCleanup) /* 9 */
#endif
#ifndef TkSendCleanup
#define TkSendCleanup \
	(tkIntPlatStubsPtr->tkSendCleanup) /* 10 */
#endif
#ifndef TkFreeXId
#define TkFreeXId \
	(tkIntPlatStubsPtr->tkFreeXId) /* 11 */
#endif
#ifndef TkpWmSetState
#define TkpWmSetState \
	(tkIntPlatStubsPtr->tkpWmSetState) /* 12 */
#endif
#ifndef TkpTestsendCmd
#define TkpTestsendCmd \
	(tkIntPlatStubsPtr->tkpTestsendCmd) /* 13 */
#endif
#endif /* X11 */

#endif /* defined(USE_TK_STUBS) && !defined(USE_TK_STUB_PROCS) */

/* !END!: Do not edit above this line. */

#undef TCL_STORAGE_CLASS
#define TCL_STORAGE_CLASS DLLIMPORT

#ifdef __CYGWIN__
    void TkFreeXId(TkDisplay *dispPtr);
    void TkFreeWindowId(TkDisplay *dispPtr, Window w);
    void TkInitXId(TkDisplay *dispPtr);
#endif

#ifdef __WIN32__
#undef TkpCmapStressed
#undef TkpSync
#define TkpCmapStressed(tkwin,colormap) (0)
#define TkpSync(display)
#endif

#endif /* _TKINTPLATDECLS */
