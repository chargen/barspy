// 13 december 2016
#include "winapi.hpp"
#include "resources.h"

#define APPTITLE L"BarSpy"

// main.cpp
extern HINSTANCE hInstance;
extern int nCmdShow;
extern HICON hDefIcon;
extern HCURSOR hDefCursor;
extern HBRUSH blackBrush;
extern HFONT hMessageFont;
// TODO rename or delete this
extern void initCommon(HINSTANCE hInst, int nCS);

// mainwin.cpp
extern HWND mainwin;
extern void openMainWindow(void);

// panic.cpp
extern void panic(const WCHAR *msg, ...);

// util.cpp
extern int classNameOf(WCHAR *classname, ...);
extern WCHAR *windowClass(HWND hwnd);
extern int windowClassOf(HWND hwnd, ...);
#define DESIREDCLASSES TOOLBARCLASSNAMEW, REBARCLASSNAMEW
#define DESIREDTOOLBAR 0
#define DESIREDREBAR 1
extern HDWP beginDeferWindowPos(int n);
extern HDWP deferWindowPos(HDWP dwp, HWND hwnd, int x, int y, int width, int height, UINT flags);
extern void endDeferWindowPos(HDWP dwp);
extern std::wstring colorToString(COLORREF color);
extern std::wstring sizeToString(SIZE size);
extern void deleteObject(HGDIOBJ obj);
extern HBRUSH createSolidBrush(COLORREF color);

// enum.cpp
extern void enumWindowTree(HWND treeview, HTREEITEM (*f)(HWND treeview, HWND window, HTREEITEM parent));

// checkmark.cpp
extern HICON hIconYes;
extern HICON hIconNo;
extern HICON hIconUnknown;
extern void initCheckmarks(void);
extern HWND newCheckmark(HWND parent, HMENU id);
extern void setCheckmarkIcon(HWND hwnd, HICON icon);
extern SIZE checkmarkSize(HWND hwnd);

// layout.cpp
class Layouter {
	HWND hwnd;
	HDC dc;
	HFONT prevfont;
	int baseX, baseY;
	TEXTMETRICW tm;
public:
	Layouter(HWND hwnd);
	~Layouter();

	int X(int x);
	int Y(int y);
	LONG InternalLeading(void);

	LONG TextWidth(const WCHAR *text, size_t len);
	LONG TextWidth(const WCHAR *text);
	LONG TextWidth(HWND hwnd);

	// specifics
	int PaddingX(void);
	int PaddingY(void);
	int WindowMarginX(void);
	int WindowMarginY(void);
	int EditHeight(void);
	int LabelYForSiblingY(int siblingY);
	int LabelHeight(void);
};
struct RowYMetrics {
	LONG TotalHeight;
	LONG LabelY;
	LONG LabelHeight;
	LONG EditY;
	LONG EditHeight;
	LONG IconY;
	LONG LabelEditY;
};
extern void rowYMetrics(struct RowYMetrics *m, Layouter *d, RECT *iconRect = NULL);
extern LONG longestTextWidth(Layouter *d, const std::vector<HWND> &hwnds);
template<typename... Ts>
extern LONG longestTextWidth(Layouter *d, HWND first, Ts... hwnds);
class Form {
	HWND parent;
	int id;
	int minEditWidth;
	bool padded;
	std::vector<HWND> labels;
	std::vector<HWND> edits;
	std::vector<HWND> icons;
	HWND firstIcon;
	void addLabel(const WCHAR *label);
	void padding(Layouter *d, LONG *x, LONG *y);
	LONG effectiveMinEditWidth(void);
	HDWP relayout(HDWP dwp, LONG x, LONG y, bool useWidth, LONG width, bool widthIsEditOnly, Layouter *d);
public:
	Form(HWND parent, int id = 100, int minEditWidth = 0);
	int ID(void);
	void SetID(int id);
	void SetMinEditWidth(int editMinWidth);
	void SetPadded(bool padded);
	void Add(const WCHAR *label);
	void AddCheckmark(const WCHAR *label);
	void SetText(int id, const WCHAR *text);
	void SetCheckmark(int id, HICON icon);
	void RowYMetrics(struct RowYMetrics *m, Layouter *d);
	SIZE MinimumSize(Layouter *d);
	HDWP Relayout(HDWP dwp, LONG x, LONG y, Layouter *d);
	HDWP RelayoutWidth(HDWP dwp, LONG x, LONG y, LONG width, Layouter *d);
	HDWP RelayoutEditWidth(HDWP dwp, LONG x, LONG y, LONG width, Layouter *d);
	int WhichRowIs(HWND edit);
	void QueueRedraw(int which);
};
class Chain {
	HWND parent;
	int id;
	int minEditWidth;
	bool padded;
	std::vector<HWND> labels;
	std::vector<HWND> edits;
	void padding(Layouter *d, LONG *x, LONG *y);
public:
	Chain(HWND parent, int id = 100, int minEditWidth = 0);
	int ID(void);
	void SetID(int id);
	void SetMinEditWidth(int editMinWidth);
	void SetPadded(bool padded);
	void Add(const WCHAR *label);
	void AddTrailingLabel(const WCHAR *label);
	void SetText(int id, const WCHAR *text);
	void RowYMetrics(struct RowYMetrics *m, Layouter *d);
	SIZE MinimumSize(Layouter *d);
	HDWP Relayout(HDWP dwp, LONG x, LONG y, Layouter *d);
};

// process.cpp
class Process {
	DWORD pid;
	HANDLE hProc;
public:
	Process(DWORD pid);
	~Process(void);

	bool Is64Bit(void);

	void *AllocBlock(size_t size);
	void FreeBlock(void *block);
	void MakeExecutable(void *block, size_t size);

	void Read(void *base, size_t off, void *buf, size_t len);
	void Write(void *base, size_t off, const void *buf, size_t len);

	void *GetModuleBase(const WCHAR *modname);
	void *GetProcAddress(void *modbase, const char *procname);

	HANDLE CreateThread(void *threadProc, void *param);

	// convenience so specific tabs can access it
	bool IsV6;
};
extern void initProcess(void);
extern Process *processFromHWND(HWND hwnd);

// prochelper.cpp
class ProcessHelper {
	Process *p;
	struct ProcessHelperPriv *priv;
	void finalizeData(void);
public:
	ProcessHelper(Process *p);
	// TODO make sure no function has () (empty argument lists)
	~ProcessHelper(void);

	void SetCode(const uint8_t *code386, size_t n386, const uint8_t *codeAMD64, size_t nAMD64);
	void AddField(const std::string &name, int type, size_t off386, size_t size386, size_t offAMD64, size_t sizeAMD64);
	template<typename T> void ReadField(const std::string &field, T *out);
	template<typename T> void WriteField(const std::string &field, T val);
	void *ReadFieldPointer(const std::string &field);
	void WriteFieldPointer(const std::string &field, void *ptr);
	void WriteFieldProcAddress(const std::string &field, void *modbase, const char *name);
	void SetExtraDataSize(size_t n);
	void *ExtraDataPtr(void);
	void *ReadExtraData(void);
	void WriteExtraData(const void *data);
	void WriteExtraData(const void *data, size_t size);
	void Run(void);
};
// field types
enum {
	fieldPointer,
	fieldATOM,
	fieldUINT,
	fieldDWORD,
	fieldHRESULT,
	fieldCOLORREF,
	fieldLONG,
	fieldInt,
	fieldBOOL,
};

// common.cpp
class Common {
	Form version;
	Form unicode;
	Chain setWindowTheme;

	Form styles;
public:
	Common(HWND parent, int idoff);

	void Reset(void);
	void Reflect(HWND hwnd, Process *p);

	SIZE MinimumSize(Layouter *d);
	HDWP Relayout(HDWP dwp, RECT *fill, Layouter *d);
};
extern std::wstring drawTextFlagsString(UINT relevant);

// getwindowtheme.cpp
extern void getWindowTheme(HWND hwnd, Process *p, WCHAR **pszSubAppName, WCHAR **pszSubIdList);

// dllgetversion.cpp
extern WCHAR *getDLLVersion(HWND hwnd, Process *p);

// tab.cpp
class Tab {
	HWND hwnd;
	HWND parent;
	int id;
	std::vector<HWND> pages;
	void rearrangeZOrder(void);
	static INT_PTR CALLBACK dlgproc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT curpagenum(void);
	HWND curpage(void);
	HDWP relayoutCurrentPage(HDWP dwp, RECT *fill, Layouter *d);
public:
	Tab(HWND parent, int id);
	HWND Add(const WCHAR *name);
	void Show(int cmd);
	bool HandleNotify(NMHDR *nm, LRESULT *lResult);
	// TODO MinimumSize
	HDWP Relayout(HDWP dwp, RECT *fill, Layouter *d);
	virtual HDWP RelayoutChild(HDWP dwp, HWND page, RECT *fill, Layouter *d) = 0;
	virtual bool OnCtlColorStatic(HDC dc, HWND hwnd, HBRUSH *brush) = 0;
};

// toolbartab.cpp
class ToolbarTab : public Tab {
	HWND generalTab;
	HWND buttonsTab;
	HWND imagelistTab;

	Form *generalCol1;
	Form *generalCol2;

	Chain *buttonCount;
	HWND buttonList;

	// data about the current toolbar
	void reset(void);
	HBRUSH buttonHighlightBrush;
	HBRUSH buttonShadowBrush;
	HBRUSH insertionPointBrush;
	HIMAGELIST normalImageList;
	HIMAGELIST hotImageList;
	HIMAGELIST pressedImageList;
	HIMAGELIST disabledImageList;
public:
	ToolbarTab(HWND parent, int id);
	void Reflect(HWND hwnd, Process *p);
	virtual HDWP RelayoutChild(HDWP dwp, HWND page, RECT *fill, Layouter *d);
	virtual bool OnCtlColorStatic(HDC dc, HWND hwnd, HBRUSH *brush);
};

// toolbargeneral.cpp
extern ProcessHelper *getToolbarGeneral(HWND hwnd, Process *p);

// loadlibrary.cpp
extern HMODULE loadLibraryProcess(Process *p, const WCHAR *name);

// freelibrary.cpp
extern void freeLibraryProcess(Process *p, HMODULE module);

// writeimglist5.cpp
extern HGLOBAL writeImageListV5(HWND hwnd, Process *p, HIMAGELIST imglist, void *pole32);

// writeimglist6.cpp
extern HGLOBAL writeImageListV6(HWND hwnd, Process *p, HIMAGELIST imglist, void *pole32);

// dumphglobal.cpp
extern uint8_t *dumpHGLOBALStreamData(Process *p, HGLOBAL hGlobal, SIZE_T *size);

// flags.cpp
extern std::wstring toolbarStyleString(HWND toolbar);
extern std::wstring toolbarExStyleString(HWND toolbar);
extern std::wstring toolbarBitmapFlagsString(HWND toolbar);
extern std::wstring drawTextFlagsString(DWORD v);
