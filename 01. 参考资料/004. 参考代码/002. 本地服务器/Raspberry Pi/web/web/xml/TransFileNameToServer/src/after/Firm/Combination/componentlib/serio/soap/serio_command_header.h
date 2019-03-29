//gsoap ns service name: serio command
//gsoap ns service namespace: http://xml.brother.co.jp/serio1.00/
//gsoap ns service location:    http://localhost


enum ns__FileFormatSelection {
    ns__FileFormatSelection__TIFF,
    ns__FileFormatSelection__JPEG,
    ns__FileFormatSelection__PDF,
    ns__FileFormatSelection__XPS,
    ns__FileFormatSelection__SPDF,
    ns__FileFormatSelection__SIPDF,
    ns__FileFormatSelection__PDFA 
};

enum ns__FileNameFixedSelection{
    ns__FileNameFixedSelection__TIFF,
};

enum ns__Selection5 {
    ns__Selection5__Highest,
    ns__Selection5__Higher,
    ns__Selection5__Normal,
    ns__Selection5__Lower,
    ns__Selection5__Lowest
};

enum ns__Selection3 {
    ns__Selection3__High,
    ns__Selection3__Normal,
    ns__Selection3__Low
};

enum ns__ScanTraySelection {
    ns__ScanTraySelection__FB,
    ns__ScanTraySelection__ADF,
    ns__ScanTraySelection__Auto
};

enum ns__ColorModeSelection{
    ns__ColorModeSelection__Mono,
    ns__ColorModeSelection__Gray,
    ns__ColorModeSelection__Color,
    ns__ColorModeSeledtion__Auto
};

enum ns__TxTypeSelection {
    ns__TxTypeSelection__FAX,
    ns__TxTypeSelection__IFAX,
    ns__TxTypeSelection__DirectSMTP,
    ns__TxTypeSelection__SMTP,
    ns__TxTypeSelection__CIFS,
    ns__TxTypeSelection__FTP,
    ns__TxTypeSelection__Scan2File,
    ns__TxTypeSelection__Scan2Image,
    ns__TxTypeSelection__Scan2Email,
    ns__TxTypeSelection__Scan2OCR
};

enum ns__FaxQualitySelection{
    ns__FaxQualitySelection__Standard,
    ns__FaxQualitySelection__Fine,
    ns__FaxQualitySelection__SuperFine,
    ns__FaxQualitySelection__Photo
};

enum ns__CopyQualitySelection{
    ns__CopyQualitySelection__Best,
    ns__CopyQualitySelection__Normal,
    ns__CopyQualitySelection__Fast,
    ns__CopyQualitySelection__Photo,
    ns__CopyQualitySelection__Receipt,
    ns__CopyQualitySelection__Text,
    ns__CopyQualitySelection__Auto,
};

enum ns__FeedTraySelection {
    ns__FeedTraySelection__Auto,
    ns__FeedTraySelection__MP,
    ns__FeedTraySelection__T1,
    ns__FeedTraySelection__T2,
    ns__FeedTraySelection__T3,
    ns__FeedTraySelection__T4,
    ns__FeedTraySelection__T5,
    ns__FeedTraySelection__T6,
    ns__FeedTraySelection__T7,
    ns__FeedTraySelection__T8
};


enum ns__LayoutSelection {
    ns__LayoutSelection__Normal,
    ns__LayoutSelection__2in1,
    ns__LayoutSelection__4in1,
    ns__LayoutSelection__9in1,
    ns__LayoutSelection__16in1,
    ns__LayoutSelection__25in1
};

enum ns__OrientationSelection{
    ns__OrientationSelection__Portrait,
    ns__OrientationSelection__Landscape
};

enum ns__IojobStatusUpdated{
    ns__IojobStatusUpdated__JobCommandReceived,
    ns__IojobStatusUpdated__JobStarted,
    ns__IojobStatusUpdated__OnePagePrinted,
    ns__IojobStatusUpdated__OneDataCommFinished
};

struct ns__HttpHeader{
    char*                                  HeaderName               1;
    char*                                  HeaderValue              1;
};
struct ns__AdditionalHttpHeaders
{
    int                                    __sizeHttpHeader;
    struct ns__HttpHeader*                 HttpHeader               1;

    /* Any Elements */
    int                                    __size__any;
    xsd__any*                              __any                    0;

};

struct ns__CallbackHttpHeaders
{
    int                                    __sizeCallbackHttpHeader;
    char**                                 CallbackHttpHeader       1;
    
    /* Any Elements */
    int                                    __size__any;
    xsd__any*                              __any                    0;

};

enum ns__DlMethod {
    ns__DlMethod__GET,
    ns__DlMethod__POST
};

enum ns__UlMethod
{
    ns__UlMethod__PUT,
    ns__UlMethod__POST
};

struct ns__LockAttr{
    @enum xsd__boolean*                    permitted                0;
    @enum xsd__boolean*                    colorEnable              0;
    
    /* Any Elements */
    int                                    __size__any;
    xsd__any*                              __any                    0;
};

struct ns__ScanAndSendLockAttr{
    @enum xsd__boolean*                    permitted                0;
    @enum xsd__boolean*                    colorEnable              0;
    
    struct ns__LockAttr*                   Scan2Media               0;
    struct ns__LockAttr*                   Scan2Remote              0;

    /* Any Elements */
    int                                    __size__any;
    xsd__any*                              __any                    0;
};

struct ns__LockInfo
{
    @enum xsd__boolean*                    permitted                0;
    @enum xsd__boolean*                    colorEnable              0;

    struct ns__LockAttr*                   Copy                     0;
    struct ns__LockAttr*                   SendFax                  0;
    struct ns__ScanAndSendLockAttr*        ScanAndSend              0;
    struct ns__LockAttr*                   DirectPrint              0;
    struct ns__LockAttr*                   PrintFax                 0;
    struct ns__LockAttr*                   PcPrint                  0;
    struct ns__LockAttr*                   WebConnectUload          0;
    struct ns__LockAttr*                   WebConnectDload          0;
    
    /* Any Elements */
    int                                    __size__any;
    xsd__any*                              __any                    0;

};

/* CM-401: Lock設定  */
struct ns__ComActivateLock
{
    char*                                  UserId                   1;
    struct ns__LockInfo                    LockInfo                 1;
    xsd__anyURI                            JobFinAckUrl             0;

    /* Any Elements */
    int                                    __size__any;
    xsd__any*                              __any                    0;
};

/* CM-402: Lock設定解除 */
struct ns__ComDeactivateLock
{
    xsd__anyURI                            JobFinAckUrl             0;

    /* Any Elements */
    int                                    __size__any;
    xsd__any*                              __any                    0;

};

/* CM-511: 利用者入力要求  */
struct ns__ComDisplayForm
{
    char*                                  Script                   1;

    /* Any Elements */
    int                                    __size__any;
    xsd__any*                              __any                    0;

};

/* CM-521: 情報報知画面表示 */
struct ns__ComDisplayInfo
{
    char*                                  Script                   1;

    /* Any Elements */
    int                                    __size__any;
    xsd__any*                              __any                    0;

};

/* CM-601: DB読み出し */
enum ns__ReadDb_ValueFormat {
    ns__ReadDb_ValueFormat__DisplayString,
    ns__ReadDb_ValueFormat__HexString
};
struct ns__ReadDbProfile
{
    char*                                  AuthInfo                 0;
    char*                                  Key                      1;
    enum ns__ReadDb_ValueFormat*           ValueFormat              0;

    /* Any Elements */
    int                                    __size__any;
    xsd__any*                              __any                    0;
};
struct ns__ReadDbProfileList
{
    int                                    __sizeReadDbProfile;
    struct ns__ReadDbProfile*              ReadDbProfile            1;
};
struct ns__ComReadDb
{
    struct ns__ReadDbProfileList           ReadDbProfiles           1;
    xsd__anyURI                            JobFinAckUrl             0;

    /* Any Elements */
    int                                    __size__any;
    xsd__any*                              __any                    0;

};

/* CM-602: DB連続読みだし */
struct ns__ComReadDbBundle
{
    struct ns__ReadDbProfile               ReadDbProfile            1;
    xsd__anyURI                            JobFinAckUrl             0;

    /* Any Elements */
    int                                    __size__any;
    xsd__any*                              __any                    0;

};

/* CM-610: DB書き込み */
struct ns__ValueType {
    int                                    __union_ValueType;
    union _ns__union_ValueType {
        char*                              Value                    1;
        char*                              HexValue                 1;
    } union_ValueType;
};

struct ns__MibDataType {
    char*                                  Key                      1;
    struct ns__ValueType                   __ValueType              1;
    char*                                  AuthInfo                 0;

};

struct ns__MibDataList
{
    int                                    __sizeMib;
    struct ns__MibDataType*                Mib                      1;
};

struct ns__ComUpdateDb
{
    struct ns__MibDataList                 DataList                 1;
    xsd__anyURI                            JobFinAckUrl             0;
    
    /* Any Elements */
    int                                    __size__any;
    xsd__any*                              __any                    0;

};

/* CM-701: iojob copy */
struct ns__ComIoCopy
{
	@enum xsd__boolean*                    strictParam              0;
	@enum xsd__boolean*                    paramCheckOnly           0;
    char*                                  UserId                   0;
    enum ns__ScanTraySelection*            ScanTray                 0;
    enum ns__ColorModeSelection*           ColorMode                0;
    enum ns__CopyQualitySelection*         CopyQuality              0;
    enum ns__Selection5*                   Density                  0;
    enum ns__Selection5*                   Brightness               0;
    enum ns__Selection5*                   Contrast                 0;
    enum ns__FeedTraySelection*            FeedTray                 0;
    int*                                   Ratio                    0;
    enum ns__LayoutSelection*              Layout                   0;
    enum ns__OrientationSelection*         LayoutOrientation        0;
    enum xsd__boolean*                     DuplexScanEnable         0;
    enum xsd__boolean*                     DuplexPrintEnable        0;
    enum xsd__boolean*                     ShortEdgeBinding         0;
    enum xsd__boolean*                     Sorting                  0;
    int*                                   NumCopies                0;
    int*                                   PrintLimit               0;

    xsd__anyURI                            CmdRecvAckUrl            0;
    xsd__anyURI                            JobStartAckUrl           0;
    xsd__anyURI                            PagePrintAckUrl          0;
    xsd__anyURI                            JobStatusAckUrl          0;
    xsd__anyURI                            JobFinAckUrl             0;
    
    /* Any Elements */
    int                                    __size__any;
    xsd__any*                              __any                    0;

};


/* CM-702: iojob fax */


struct ns__FaxProperty {
    char*                                  Destination              1;
    
    /* Any Elements */
    int                                    __size__any;
    xsd__any*                              __any                    0;
};

struct ns__IfaxProperty {
    char*                                  Destination              1;
    /* Any Elements */
    int                                    __size__any;
    xsd__any*                              __any                    0;
};

struct ns__DirectSmtpProperty {
    char*                                  Destination              1;
    /* Any Elements */
    int                                    __size__any;
    xsd__any*                              __any                    0;
};

struct ns__FaxProfileChoices
{
    int                                    __union;
    union ns__FaxProfile {
        struct ns__FaxProperty             Fax                      1;
        struct ns__IfaxProperty            Ifax                     1;
        struct ns__DirectSmtpProperty      DirectSMTP               1;
        xsd__any                           __any;
    }FaxProfile;
};

struct ns__FaxProfiles
{
    int                                    __size__FaxProfiles;
    struct ns__FaxProfileChoices*          __FaxProfiles            1;
};

struct ns__ComIoSendFax
{
    char*                                  UserId                   0;
    struct ns__FaxProfiles                 FaxProfiles              1;
    enum ns__ScanTraySelection*            ScanTray                 0;
    enum ns__ColorModeSelection*           ColorMode                0;
    enum ns__FaxQualitySelection*          FaxQuality               0;
    enum xsd__boolean*                     DuplexScanEnable         0;
    enum xsd__boolean*                     ShortEdgeBinding         0;

    xsd__anyURI                            CmdRecvAckUrl            0;
    xsd__anyURI                            JobStartAckUrl           0;
    xsd__anyURI                            IoDataCommAckUrl         0;
    xsd__anyURI                            JobStatusAckUrl          0;
    xsd__anyURI                            JobFinAckUrl             0;
    /* Any Elements */
    int                                    __size__any;
    xsd__any*                              __any                    0;
};




/* CM-703: iojob: Scan and send */
struct ns__SmtpProperty {
    int                                    __sizeDestination;
    char**                                 Destination              1;
    char*                                  Subject                  0;
    char*                                  MsgBody                  0;
    /* Any Elements */
    int                                    __size__any;
    xsd__any*                              __any                    0;
};

enum ns__CifsParams_AuthMethod {
    ns__CifsParams_AuthMethod__Auto,
    ns__CifsParams_AuthMethod__Kerberos,
    ns__CifsParams_AuthMethod__NTLMv2
};

struct ns__CifsParams {
    char*                                  Host                     1;
    char*                                  StoreDir                 1;
    char*                                  FileName                 1;
    enum ns__CifsParams_AuthMethod         AuthMethod               1;
    char*                                  User                     1;
    char*                                  Password                 1;
    char*                                  KerberosServer           0;
    
    /* Any Elements */
    int                                    __size__any;
    xsd__any*                              __any                    0;
};

struct ns__CifsProperty {
    struct ns__CifsParams*                 CifsParams               0;
};

struct ns__FtpParams {
    char*                                  FileName                 1;
    char*                                  Host                     1;
    char*                                  User                     1;
    char*                                  Password                 1;
    char*                                  StoreDir                 1;
    enum xsd__boolean                      PassiveMode              1;
    char*                                  PortNum                  1;
    
    /* Any Elements */
    int                                    __size__any;
    xsd__any*                              __any                    0;
};

struct ns__FtpProperty {
    struct ns__FtpParams*                  FtpParams                0;
};

struct ns__Scan2fileProperty {
    char*                                  Destination              1;
    /* Any Elements */
    int                                    __size__any;
    xsd__any*                              __any                    0;
};

struct ns__Scan2emailProperty {
    char*                                  Destination              1;
    /* Any Elements */
    int                                    __size__any;
    xsd__any*                              __any                    0;
};

struct ns__Scan2ocrProperty {
    char*                                  Destination              1;
    /* Any Elements */
    int                                    __size__any;
    xsd__any*                              __any                    0;
};

struct ns__TxProfileChoices {
    int                                    __union;
    union ns__TxProfile {
        struct ns__SmtpProperty            Smtp                     1;
        struct ns__CifsProperty            Cifs                     1;
        struct ns__FtpProperty             Ftp                      1;
        struct ns__Scan2fileProperty       Scan2file                1;
        struct ns__Scan2emailProperty      Scan2email               1;
        struct ns__Scan2ocrProperty        Scan2ocr                 1;
        xsd__any                           __any                    0;
    }TxProfile;
};


struct ns__TxProfiles
{
    int                                    __size__TxProfiles;
    struct ns__TxProfileChoices*           __TxProfiles             1;
};


enum ns__UloadResolution {
    ns__UloadResolution__High,
    ns__UloadResolution__Normal,
    ns__UloadResolution__Low,
    ns__UloadResolution__600,
    ns__UloadResolution__400,
    ns__UloadResolution__300,
    ns__UloadResolution__200,
    ns__UloadResolution__150,
    ns__UloadResolution__100,
    ns__UloadResolution__200x100,
    ns__UloadResolution__Auto,
};

enum ns__ScanAndUloadScansize
{
    ns__ScanAndUloadScansize__A3,
    ns__ScanAndUloadScansize__A4,
    ns__ScanAndUloadScansize__A5,
    ns__ScanAndUloadScansize__A6,
    ns__ScanAndUloadScansize__B4,
    ns__ScanAndUloadScansize__B5,
    ns__ScanAndUloadScansize__B6,
    ns__ScanAndUloadScansize__Legal,
    ns__ScanAndUloadScansize__Letter,
    ns__ScanAndUloadScansize__Hagaki,
    ns__ScanAndUloadScansize__Executive,
    ns__ScanAndUloadScansize__4times6,
    ns__ScanAndUloadScansize__PhotoL,
    ns__ScanAndUloadScansize__Photo2L,
    ns__ScanAndUloadScansize__Ledger,
    ns__ScanAndUloadScansize__BusinessCard,
};


struct ns__ComIoScanAndSend
{
	@enum xsd__boolean*                    strictParam              0;
	@enum xsd__boolean*                    paramCheckOnly           0;
    char*                                  UserId                   0;
    struct ns__TxProfiles                  TxProfiles               1;
    enum ns__ScanTraySelection*            ScanTray                 0;
    enum ns__ColorModeSelection*           ColorMode                0;
    enum ns__UloadResolution*              Resolution               0;
    enum ns__ScanAndUloadScansize*         DocSize                  0;
    enum ns__Selection5*                   Density                  0;
    enum ns__Selection5*                   Brightness               0;
    enum ns__Selection3*                   JpgQuality               0;
    enum ns__FileFormatSelection*          FileType                 0;
    enum xsd__boolean*                     DuplexScanEnable         0;
    enum xsd__boolean*                     ShortEdgeBinding         0;
    enum xsd__boolean*                     FileNameFixed         	0;    
    
    xsd__anyURI                            CmdRecvAckUrl            0;
    xsd__anyURI                            JobStartAckUrl           0;
    xsd__anyURI                            IoDataCommAckUrl         0;
    xsd__anyURI                            JobStatusAckUrl          0;
    xsd__anyURI                            JobFinAckUrl             0;
    xsd__anyURI                            FileNameAckUrl           0;
    
    /* Any Elements */
    int                                    __size__any;
    xsd__any*                              __any                    0;
};


/* CM-704: iojob usb-print */
struct ns__ComIoDirectPrint
{
    char*                                  UserId                   0;
    char*                                  FileName                 0;
    int*                                   PrintLimit               0;

    xsd__anyURI                            CmdRecvAckUrl            0;
    xsd__anyURI                            JobStartAckUrl           0;
    xsd__anyURI                            PagePrintAckUrl          0;
    xsd__anyURI                            JobStatusAckUrl          0;
    xsd__anyURI                            JobFinAckUrl             0;

    /* Any Elements */
    int                                    __size__any;
    xsd__any*                              __any                    0;
};

/* CM-705: iojob memory-fax-print */
struct ns__ComIoPrintFaxmem
{
    char*                                  UserId                   0;
    char*                                  Target                   0;
    int*                                   PrintLimit               0;
    
    xsd__anyURI                            CmdRecvAckUrl            0;
    xsd__anyURI                            JobStartAckUrl           0;
    xsd__anyURI                            PagePrintAckUrl          0;
    xsd__anyURI                            JobStatusAckUrl          0;
    xsd__anyURI                            JobFinAckUrl             0;

    /* Any Elements */
    int                                    __size__any;
    xsd__any*                              __any                    0;
};

/* CM-706: iojob job-spooling */
struct ns__ComIoPrintSpool
{
    char*                                  UserId                   0;
    char*                                  Target                   0;
    int*                                   PrintLimit               0;

    xsd__anyURI                            CmdRecvAckUrl            0;
    xsd__anyURI                            JobStartAckUrl           0;
    xsd__anyURI                            PagePrintAckUrl          0;
    xsd__anyURI                            JobStatusAckUrl          0;
    xsd__anyURI                            JobFinAckUrl             0;

    /* Any Elements */
    int                                    __size__any;
    xsd__any*                              __any                    0;
};

/* CM-707: iojob PCprint */
struct ns__ComIoPcPrint
{
    char*                                  UserId                   0;
    char*                                  Target                   0;
    int*                                   PrintLimit               0;
    enum xsd__boolean*                     ColorEnable              0;
    
    xsd__anyURI                            CmdRecvAckUrl            0;
    xsd__anyURI                            JobStartAckUrl           0;
    xsd__anyURI                            PagePrintAckUrl          0;
    xsd__anyURI                            JobStatusAckUrl          0;
    xsd__anyURI                            JobFinAckUrl             0;

    /* Any Elements */
    int                                    __size__any;
    xsd__any*                              __any                    0;
};

/* CM-708 MediaRead and Upload */
struct ns__SkipView{
    @char*                                 name                     1;
};

struct ns__SkipViewList{
    int                                    __sizeSkipView;
    struct ns__SkipView*                   SkipView                 1;
};

struct ns__UloadFilesInMedia
{
    int                                    __sizeFileName;
    xsd__anyURI*                           FileName                 1;
};

struct ns__ComIoUloadFromMedia
{
    struct ns__UloadFilesInMedia           UloadFiles               1;
    struct ns__SkipViewList*               SkipViewList             0;
    xsd__anyURI                            CmdRecvAckUrl            0;
    xsd__anyURI                            JobStartAckUrl           0;
    xsd__anyURI                            FileInfoAckUrl           1;
    xsd__anyURI                            EachUloadAckUrl          1;
    xsd__anyURI                            JobFinAckUrl             0;

    /* Any Elements */
    int                                    __size__any;
    xsd__any*                              __any                    0;
};

/* CM-709 Scan and upload */
enum ns__UlFileType {
    ns__UlFileType__JPEG,
    ns__UlFileType__PDF
};

enum ns__ScanModeType {
    ns__ScanModeType__Copy,
    ns__ScanModeType__Scan
};

/* ink/Laser共通のコピー品質パラメータ */
enum ns__CopyQualitySelection2 {
    ns__CopyQualitySelection2__Best,
    ns__CopyQualitySelection2__Normal,
    ns__CopyQualitySelection2__Fast,
    ns__CopyQualitySelection2__Photo,
    ns__CopyQualitySelection2__Receipt,
    ns__CopyQualitySelection2__Text,
    ns__CopyQualitySelection2__Auto,
};

enum ns__ScanMarginSelection {
    ns__ScanMarginSelection__Auto,
    ns__ScanMarginSelection__0mm,
    ns__ScanMarginSelection__1mm,
    ns__ScanMarginSelection__2mm,
    ns__ScanMarginSelection__3mm,
};

struct ns__ComIoScanAndUload
{
    struct ns__SkipViewList*               SkipViewList             0;
    enum ns__ScanTraySelection*            ScanTray                 0;
    enum ns__ColorModeSelection*           ColorMode                0;
    enum ns__UloadResolution*              Resolution               0;
    enum ns__ScanModeType*                 ScanMode                 0;
    enum ns__CopyQualitySelection2*        CopyQuality              0;
    enum ns__ScanMarginSelection*          Margin                   0;
    enum ns__ScanAndUloadScansize*         DocSize                  0;
    enum ns__Selection5*                   Density                  0;
    enum ns__Selection5*                   Brightness               0;
    enum ns__Selection3*                   JpgQuality               0;
    enum ns__UlFileType*                   UlFileType               0;
    enum xsd__boolean*                     DuplexScanEnable         0;
    enum xsd__boolean*                     ShortEdgeBinding         0;
    
    xsd__anyURI                            CmdRecvAckUrl            0;
    xsd__anyURI                            JobStartAckUrl           0;
    xsd__anyURI                            FileInfoAckUrl           1;
    xsd__anyURI                            EachUloadAckUrl          1;
    xsd__anyURI                            JobFinAckUrl             0;

    /* Any Elements */
    int                                    __size__any;
    xsd__any*                              __any                    0;
};

/* CM-710 Download and print */
struct ns__DlPrtFile
{
    char*                                  Url                      1;
    char*                                  TplVariableName          0;
    char*                                  TplVariableValue         0;
    int*                                   NumCopies                0;
    
    /* Any Elements */
    int                                    __size__any;
    xsd__any*                              __any                    0;

};

struct ns__DlPrtFileList
{
    int                                    __sizeDlPrtFile;
    struct ns__DlPrtFile*                  DlPrtFile                1;
};


enum ns__DlPrintQuality
{
    ns__DlPrintQuality__Normal,
    ns__DlPrintQuality__Photo,
    ns__DlPrintQuality__Text,
    ns__DlPrintQuality__FinePhoto,
    ns__DlPrintQuality__Fine,
    ns__DlPrintQuality__Auto,
    ns__DlPrintQuality__Receipt
};

enum ns__DlPrintPapertype
{
    ns__DlPrintPapertype__Plain,
    ns__DlPrintPapertype__Inkjet,
    ns__DlPrintPapertype__BP61,
    ns__DlPrintPapertype__BP71,
    ns__DlPrintPapertype__Thin,
    ns__DlPrintPapertype__Recycled,
    ns__DlPrintPapertype__Other
};

enum ns__DlPrintPapersize
{
    ns__DlPrintPapersize__A3,
    ns__DlPrintPapersize__A4,
    ns__DlPrintPapersize__A5,
    ns__DlPrintPapersize__A5L,
    ns__DlPrintPapersize__A6,
    ns__DlPrintPapersize__B4,
    ns__DlPrintPapersize__B5,
    ns__DlPrintPapersize__B6,
    ns__DlPrintPapersize__Legal,
    ns__DlPrintPapersize__Letter,
    ns__DlPrintPapersize__Executive,
    ns__DlPrintPapersize__4times6,
    ns__DlPrintPapersize__5times7,
    ns__DlPrintPapersize__PhotoL,
    ns__DlPrintPapersize__Photo2L,
    ns__DlPrintPapersize__Hagaki,
    ns__DlPrintPapersize__Ledger,
    ns__DlPrintPapersize__Folio,
};

struct ns__ComIoDloadAndPrint
{
    struct ns__DlPrtFileList               DlPrtFiles               1;
    struct ns__AdditionalHttpHeaders*      AdditionalHttpHeaders    0;
    struct ns__CallbackHttpHeaders*        CallbackHttpHeaders      0;
    enum ns__DlMethod*                     DlMethod                 0;
    enum ns__ColorModeSelection*           ColorMode                0;
    enum ns__DlPrintQuality*               PrintQuality             0;
    enum ns__DlPrintPapertype*             PaperType                0;
    enum ns__DlPrintPapersize*             PaperSize                0;
    enum xsd__boolean*                     BorderlessPrint          0;
    enum xsd__boolean*                     DuplexPrintEnable        0;
    enum xsd__boolean*                     ShortEdgeBinding         0;
    char*                                  Template                 0;

    xsd__anyURI                            CmdRecvAckUrl            0;
    xsd__anyURI                            JobStartAckUrl           0;
    xsd__anyURI                            PagePrintAckUrl          0;
    xsd__anyURI                            EachDloadAckUrl          0;
    xsd__anyURI                            JobFinAckUrl             0;
    
    /* Any Elements */
    int                                    __size__any;
    xsd__any*                              __any                    0;

};

/* CM-710a Download and print2 (bhS13-) */

enum ns__CpProtocol {
    ns__CpProtocol__BrotherCP,
};

struct ns__DlFileInfo
{
    char*                                  FileId                   1;
    enum ns__CpProtocol*                   Protocol                 0;
    char*                                  DlFileType               0;
    int*                                   NumCopies                0;
    char*                                  FileName                 0;
    
    /* Any Elements */
    int                                    __size__any;
    xsd__any*                              __any                    0;

};

struct ns__DlFileInfoList
{
    int                                    __sizeDlFileInfo;
    struct ns__DlFileInfo*                 DlFileInfo               1;
};

enum ns__PrintMode {
    ns__PrintMode__Print,
    ns__PrintMode__Copy
};

enum ns__ScalingMethod {
    ns__ScalingMethod__Manual,
    ns__ScalingMethod__AutoCropping,
    ns__ScalingMethod__AutoFitting
};

struct ns__Scaling {
    @enum ns__ScalingMethod                method                   1;
    @int*                                  ratio                    0;
    /* Any Elements */
    int                                    __size__any;
    xsd__any*                              __any                    0;    
};

struct ns__ComIoDloadAndPrint2
{
    struct ns__DlFileInfoList              DlFileInfoList           1;
    struct ns__SkipViewList*               SkipViewList             0;
    enum ns__ColorModeSelection*           ColorMode                0;
    enum ns__DlPrintQuality*               PrintQuality             0;
    enum ns__DlPrintPapertype*             PaperType                0;
    enum ns__DlPrintPapersize*             PaperSize                0;
    enum xsd__boolean*                     BorderlessPrint          0;
    enum xsd__boolean*                     DuplexPrintEnable        0;
    enum xsd__boolean*                     ShortEdgeBinding         0;
    struct ns__Scaling*                    Scaling                  0;
    enum ns__PrintMode*                    PrintMode                0;

    xsd__anyURI                            CmdRecvAckUrl            0;
    xsd__anyURI                            JobStartAckUrl           0;
    xsd__anyURI                            PagePrintAckUrl          0;
    xsd__anyURI                            EachDloadAckUrl          0;
    xsd__anyURI                            DlFileInfoAckUrl         0;
    xsd__anyURI                            JobFinAckUrl             0;

    /* Any Elements */
    int                                    __size__any;
    xsd__any*                              __any                    0;
};

/* CM-711 Download to media */
struct ns__Dl2MediaFile{
    char*                                  Url                      1;
    char*                                  FileName                 1;
    char*                                  TplVariableName          0;
    char*                                  TplVariableValue         0;
    
    /* Any Elements */
    int                                    __size__any;
    xsd__any*                              __any                    0;
};

struct ns__Dl2MediaFileList
{
    int                                    __sizeDl2MediaFile;
    struct ns__Dl2MediaFile*               Dl2MediaFile             1;
};

struct ns__ComIoDloadToMedia
{
    struct ns__Dl2MediaFileList            Dl2MediaFiles            1;
    struct ns__AdditionalHttpHeaders*      AdditionalHttpHeaders    0;
    struct ns__CallbackHttpHeaders*        CallbackHttpHeaders      0;
    enum ns__DlMethod*                     DlMethod                 0;
    char*                                  Template                 0;
    xsd__anyURI                            CmdRecvAckUrl            0;
    xsd__anyURI                            JobStartAckUrl           0;
    xsd__anyURI                            EachDloadAckUrl          0;
    xsd__anyURI                            JobFinAckUrl             0;
    /* Any Elements */
    int                                    __size__any;
    xsd__any*                              __any                    0;
};

/* CM-711a Download to media 2 (bhS13-) */

struct ns__ComIoDloadToMedia2
{
    struct ns__DlFileInfoList              DlFileInfoList           1;
    struct ns__SkipViewList*               SkipViewList             0;

    xsd__anyURI                            CmdRecvAckUrl            0;
    xsd__anyURI                            JobStartAckUrl           0;
    xsd__anyURI                            EachDloadAckUrl          0;
    xsd__anyURI                            DlFileInfoAckUrl         0;
    xsd__anyURI                            JobFinAckUrl             0;
    
    /* Any Elements */
    int                                    __size__any;
    xsd__any*                              __any                    0;
};

/* CM-712 IoSendFromMedia */
struct ns__ComIoSendFromMedia 
{
    struct ns__TxProfiles                  TxProfiles               1;
    struct ns__UloadFilesInMedia           UloadFiles               1;
    xsd__anyURI                            CmdRecvAckUrl            0;
    xsd__anyURI                            JobStartAckUrl           0;
    xsd__anyURI                            IoDataCommAckUrl         0;
    xsd__anyURI                            JobFinAckUrl             0;

    
    /* Any Elements */
    int                                    __size__any;
    xsd__any*                              __any                    0;
};

/* CM-750 Upload開始 */
struct ns__UlFile
{
    char*                                  FileId                   1;
    xsd__anyURI                            DestUrl                  1;
    int                                    ContentLength            1;
    
    /* Any Elements */
    int                                    __size__any;
    xsd__any*                              __any                    0;
};

struct ns__ComIoStartUload
{
    struct ns__UlFile                      UlFile                   1;
    struct ns__AdditionalHttpHeaders*      AdditionalHttpHeaders    0;
    struct ns__CallbackHttpHeaders*        CallbackHttpHeaders      0;
    enum ns__UlMethod                      UlMethod                 1;
    char*                                  Template                 0;

    /* Any Elements */
    int                                    __size__any;
    xsd__any*                              __any                    0;
};

/* CM-751 Upload処理継続 */
enum ns__ContinueChoices
{
    ns__ContinueChoices__Upload,
    ns__ContinueChoices__Dwload,
    ns__ContinueChoices__Cancel,
    ns__ContinueChoices__ExecutableError
};

struct ns__ComIoContinueUloading
{
    enum ns__ContinueChoices               Continue                 1;
    char*                                  ServerErrorMsg           0;

    /* Any Elements */
    int                                    __size__any;
    xsd__any*                              __any                    0;
};

struct ns__DlFile
{
    char*                                  SrcUrl                   1;
};
/* CM-760: Download開始 */
struct ns__ComIoStartDload
{
    struct ns__DlFile                      DlFile                   1;
    struct ns__AdditionalHttpHeaders*      AdditionalHttpHeaders    0;
    enum ns__DlMethod                      DlMethod                 1;
    char*                                  RequestBody              0;

    /* Any Elements */
    int                                    __size__any;
    xsd__any*                              __any                    0;
};

/* CM-761 Dwload処理継続 */
struct ns__ComIoContinueDloading
{
    enum ns__ContinueChoices               Continue                 1;
    char*                                  ServerErrorMsg           0;

    /* Any Elements */
    int                                    __size__any;
    xsd__any*                              __any                    0;
};

/* CM-760: Download開始 */
struct ns__ComIoStartBrotherCP
{
    char*                                  ConvertId                1;

    /* Any Elements */
    int                                    __size__any;
    xsd__any*                              __any                    0;
};

/* CM-xxx: iojob get-latest-result */
struct ns__ComLastJobResultReq
{
    xsd__anyURI                            JobFinAckUrl             0;

    /* Any Elements */
    int                                    __size__any;
    xsd__any*                              __any                    0;
};

/* CM-xxx: iojob get-latest-result */
enum ns__StatusType
{
    ns__StatusType__prtAlert,
};

struct ns__StatusTypes
{
    int                                    __sizeStatusType;
    enum ns__StatusType*                   StatusType               1;
};

struct ns__ComNotifyDevStatus
{
    struct ns__StatusTypes*                StatusTypes              0;
    xsd__anyURI                            CmdRecvAckUrl            0;
    xsd__anyURI                            JobStartAckUrl           0;
    xsd__anyURI                            JobStatusAckUrl          0;
    xsd__anyURI                            JobFinAckUrl             0;

    /* Any Elements */
    int                                    __size__any;
    xsd__any*                              __any                    0;
};

/* CM-800 Session 終了指令 */
struct ns__ComCloseSession {
    xsd__anyURI                            JobFinAckUrl             0;
    
    /* Any Elements */
    int                                    __size__any;
    xsd__any*                              __any                    0;

};

/* CM-810 Wait */
struct ns__ComWait {
    int                                    Time                     1;
    xsd__anyURI                            JobFinAckUrl             0;
    
    /* Any Elements */
    int                                    __size__any;
    xsd__any*                              __any                    0;

};

/* CM-XXX GetLastUserLog */
struct ns__ComGetLastUserLog {
    xsd__anyURI                            JobFinAckUrl             0;
    
    /* Any Elements */
    int                                    __size__any;
    xsd__any*                              __any                    0;
};

/* CM-xxx IoContinue */
struct ns__ComIoContinue {
    int                                    UserChoice               1;
    int                                    PausedReason             1;
    /* Any Elements */
    int                                    __size__any;
    xsd__any*                              __any                    0;
};

/* CM-713 IoUloadRxFax */
struct ns__ComIoUloadRxFax{
    int                                    DataId                   1;
    enum ns__FileFormatSelection*          UlFileType               0;
    struct ns__SkipViewList*               SkipViewList             0;
    xsd__anyURI                            CmdRecvAckUrl            0;
    xsd__anyURI                            JobStartAckUrl           0;
    xsd__anyURI                            FileInfoAckUrl           1;
    xsd__anyURI                            EachUloadAckUrl          1;
    xsd__anyURI                            JobFinAckUrl             0;

    /* Any Elements */
    int                                    __size__any;
    xsd__any*                              __any                    0;
};

/* CM-604 GetRxFaxList */
struct ns__ComGetRxFaxList {
    xsd__anyURI                            JobFinAckUrl             0;

    /* Any Elements */
    int                                    __size__any;
    xsd__any*                              __any                    0;

};

/* CM-621 DeleteRxFax */
struct ns__ComDeleteRxFax {
    int                                    DataId                   1;
    xsd__anyURI                            JobFinAckUrl             0;

    /* Any Elements */
    int                                    __size__any;
    xsd__any*                              __any                    0;
};


struct ns__CommandType
{
    int                                    __union_CommandType;
    union _ns__union_CommandType
    {
        struct ns__ComActivateLock         ActivateLock             1; /* CM-401 */
        struct ns__ComDeactivateLock       DeactivateLock           1; /* CM-402 */
        struct ns__ComDisplayForm          DisplayForm              1; /* CM-511 */
        struct ns__ComDisplayInfo          DisplayInfo              1; /* CM-521 */
        struct ns__ComReadDb               ReadDb                   1; /* CM-601 */
        struct ns__ComReadDbBundle         ReadDbBundle             1; /* CM-602 */
        struct ns__ComGetRxFaxList         GetRxFaxList             1; /* CM-604 */
        struct ns__ComUpdateDb             UpdateDb                 1; /* CM-610 */
        struct ns__ComDeleteRxFax          DeleteRxFax              1; /* CM-621 */
        struct ns__ComIoCopy               IoCopy                   1; /* CM-701 */
        struct ns__ComIoSendFax            IoSendFax                1; /* CM-702 */
        struct ns__ComIoScanAndSend        IoScanAndSend            1; /* CM-703 */
        struct ns__ComIoDirectPrint        IoDirectPrint            1; /* CM-704 */
        struct ns__ComIoPrintFaxmem        IoPrintFaxmem            1; /* CM-705 */
        struct ns__ComIoPrintSpool         IoPrintSpool             1; /* CM-706 */
        struct ns__ComIoPcPrint            IoPcPrint                1; /* CM-707 */
        struct ns__ComIoUloadFromMedia     IoUloadFromMedia         1; /* CM-708 */
        struct ns__ComIoScanAndUload       IoScanAndUload           1; /* CM-709 */
        struct ns__ComIoDloadAndPrint      IoDloadAndPrint          1; /* CM-710 */
        struct ns__ComIoDloadAndPrint2     IoDloadAndPrint2         1; /* CM-710a */
        struct ns__ComIoDloadToMedia       IoDloadToMedia           1; /* CM-711 */
        struct ns__ComIoDloadToMedia2      IoDloadToMedia2          1; /* CM-711a */
        struct ns__ComIoSendFromMedia      IoSendFromMedia          1; /* CM-712 */
        struct ns__ComIoUloadRxFax         IoUloadRxFax             1; /* CM-713 */
        struct ns__ComIoStartUload         IoStartUload             1; /* CM-750 */
        struct ns__ComIoContinueUloading   IoContinueUloading       1; /* CM-751 */
        struct ns__ComIoStartDload         IoStartDload             1; /* CM-760 */
        struct ns__ComIoContinueDloading   IoContinueDloading       1; /* CM-761 */
        struct ns__ComIoStartBrotherCP     IoStartBrotherCP         1; /* CM-xxx */
        struct ns__ComLastJobResultReq     LastJobResultReq         1; /* CM-xxx */
        struct ns__ComNotifyDevStatus      NotifyDevStatus          1; /* CM-xxx */
        struct ns__ComCloseSession         CloseSession             1; /* CM-800 */
        struct ns__ComWait                 Wait                     1; /* CM-810 */
        struct ns__ComGetLastUserLog       GetLastUserLog           1; /* CM-XXX */
        struct ns__ComIoContinue           IoContinue               1; /* CM-xxx */
        char*                              PjlCommand               1; /* TBD */

        xsd__any                           __any                    0;
    } union_CommandType;
};




struct ns__Command
{
    int                                    __size__CommandType;
    struct ns__CommandType*                __CommandType            1;
};


///////////////////////////////////////////////
// SerioCommands 分解用
///////////////////////////////////////////////
struct ns__SerioCommands
{
    @char*                                 version                  1;
    int                                    __size__Commands;
    XML*                                   __Commands               1;
//  char**                                 __Commands               1;
    /**
     Note: 
     + gsoap v.2.7.17のバグ
       - XML* のデータ型で定義すると、日本語の処理に失敗する (2010-11-22)。
         文字列情報の喪失。
       - XML* 以外のデータ型で定義すると、HTMLエスケープ文字が保持されない。
         ツリー構造情報の喪失。
     + 日本語処理を犠牲にし、XML*型で定義することとする。

     */
    
};

///////////////////////////////////////////////
// Event
///////////////////////////////////////////////

/* EV-201 起動要求受信 */
struct ns__Value {
    @int*                                  quantity                 0;
    char*                                  __text                   1;
};

struct  ns__KeyValueData
{
    @char*                                 attr                     0;
    char*                                  Key                      1;
    struct ns__Value                       Value                    1;
};

struct  ns__KeyValueDataArray
{
    int                                    __sizeKeyValueData;
    struct  ns__KeyValueData*              KeyValueData             1;
};

enum ns__AuthAgent
{
    ns__AuthAgent__SecureFunctionLock,
    ns__AuthAgent__BrotherSolutionInterface,
    ns__AuthAgent__ActiveDirectory,
};

struct ns__EventUserId
{
    char*                                  __item                   0;
    @enum ns__AuthAgent*                   authAgent                0;
};

struct ns__EvLaunchReqReceived
{
    struct ns__EventUserId*                UserId                   0;
    struct ns__KeyValueDataArray*          LaunchArguments          0;
};


/* EV-401 Lock設定完了通知 */
struct  ns__EvLockDone
{
    int                                    ErrorInfo                1;
};


/* EV-511 利用者入力情報通知 */
struct  ns__EvUserInput
{
    struct  ns__KeyValueDataArray*         UserInputValues          0;
    int                                    ErrorInfo                1;
};


/* EV-601 DB読み出し応答 */
struct ns__DbReadResult
{
    char*                                  Key                      1;
 /* char*                                  Value                    0; */
    struct ns__ValueType*                  __ValueType              0;
    int*                                   ReadError                0;
};
struct ns__DbReadResultList
{
    int                                    __sizeReadResult;
    struct ns__DbReadResult*               ReadResult;
};
struct ns__EvDbReadDone
{
    int                                    ErrorInfo                1;
    struct ns__DbReadResultList*           ReadResults              0;
};


/* EV-610 DB書込応答 */
struct ns__DbUpdateResult
{
    char*                                  Key                      1;
    int*                                   UpdateError              0;
};
struct ns__DbUpdateResultList
{
    int                                    __sizeUpdateResult;
    struct ns__DbUpdateResult*             UpdateResult             1;
};
struct  ns__EvDbUpdateDone
{
    int                                    ErrorInfo                1;
    struct ns__DbUpdateResultList*         UpdateResults            0;
};


/* EV-731 IO Job 受付完了通知 */
struct ns__EvJobReceived
{
    /* no elements */
};

/* EV-732 Io Job開始通知 */
struct ns__EvJobStarted
{
    /* no elements */
};

/* EV-733 IO Job印刷頁毎通知 */
struct ns__EvJobOnePagePrinted
{
    /* no elements */
};

/* EV-734 IO Job Data送受信完了通知 */
struct ns__EvJobDataCommDone
{
	char*                                  FileNam                   0;
};

/* EV-735 IO Job Upload毎通知 */
struct ns__UloadFile
{
    char*                                  FileId                   1;
    int                                    FileSize                 1;
    enum ns__UlFileType*                   FileType                 0;
};
struct ns__UloadFileList 
{
    int                                    __sizeUloadFile;
    struct ns__UloadFile*                  UloadFile                1;
};
struct ns__EvUlImgIdentified
{
    struct ns__UloadFileList               UloadFiles               1;
};
/* EV-736: Download毎通知 */
struct ns__DloadFile
{
    char*                                  FileId                   1;
};
struct ns__DloadFileList
{
    int                                    __sizeDloadFile;
    struct ns__DloadFile*                  DloadFile                1;
};
struct ns__EvDlImgIdentified
{
    struct ns__DloadFileList               DloadFiles               1;
};

/* EV-737 IO Job Upload 毎通知 */
struct ns__CallbackResHttpHeaders
{
    int                                    __sizeHttpHeader;
    struct ns__HttpHeader*                 HttpHeader;
};

struct ns__EvJobOneFileUloaded
{
    char*                                  ResponseStatus           1;
    struct ns__CallbackResHttpHeaders*     CallbackResHttpHeaders   0;
    char*                                  MsgXml                   0;
};

/* EV-739 IO Job Download 毎通知 */
struct ns__EvJobOneFileDloaded
{
    /* No elements */
};

/* EV-xxx Device 状態通知 */
struct ns__StsItem
{
    int                                    __item                   1;
    @char*                                 name                     0;
};

struct ns__Status
{
    int                                    __sizeStsItem;
    struct ns__StsItem*                    StsItem                  1;
    @int*                                  priority                 0;
};

struct ns__Statuses
{
    int                                    __sizeStatus;
    struct ns__Status*                     Status                   1;
};

struct ns__EvDeviceStatusChanged
{
    struct ns__Statuses                    Statuses                 1;
};

/* EV-xxx IO Job 状態通知 */
struct ns__UserOptions
{
    int                                    __sizeUserOption;
    int*                                   Option                   1;
};

struct ns__EvJobStatusChanged
{
    @int                                   status                   1;
    @int*                                  reason                   0;
    struct ns__UserOptions*                UserOptionIndex          0;
    int*                                   ReasonDetail             0;
    struct ns__Status*                     Status                   0;
    char*                                  Description              0;
};

/* EV-740 IO Job 結果通知 */
struct ns__ValueForEachPlane
{
    int*                                   Cyan                     0;
    int*                                   Magenta                  0;
    int*                                   Yellow                   0;
    int                                    Black                    1;
};
struct ns__EvJobDone
{
    int                                    ErrorInfo                1;
    char*                                  ErrorDetail              0;
    char*                                  ServerErrorMsg           0;
    enum xsd__boolean*                     ExecutableErr            0;
    char*                                  JobName                  0;
    struct ns__EventUserId*                UserId                   0;
    char*                                  Timestamp                0;
    int*                                   NumPages                 0;
    int*                                   NumColorPages            0;
    struct ns__ValueForEachPlane*          NumImages                0;
    struct ns__ValueForEachPlane*          MeanCoverage             0;
};

/* EV-xxx */
struct ns__EvNotificationAborted
{
    int                                    ErrorInfo                1;
};

/* EV-800 */
struct ns__EvSessionClosed
{
    int                                    ErrorInfo                1;
};

/* EV-810 */
struct ns__EvWaitTimeOut
{
    int                                    ErrorInfo                1;
};

/* EV-XXX */
enum ns__LogItemType {
    ns__LogItemType__PaperSize,
    ns__LogItemType__DocSize,
    ns__LogItemType__DuplexPrintEnable,
    ns__LogItemType__DuplexScanEnable,
    ns__LogItemType__NumMonoPages,
    ns__LogItemType__NumColorPages,
    ns__LogItemType__NumSendPages,
    ns__LogItemType__ColorMode,
    ns__LogItemType__FaxQuality,
    ns__LogItemType__ScanTo,
    ns__LogItemType__Resolution,
    ns__LogItemType__Destination,
    ns__LogItemType__FileName,
    ns__LogItemType__JobResult
};

enum ns__LogFuncType {
    ns__LogFuncType__Copy,
    ns__LogFuncType__DirectPrint,
    ns__LogFuncType__SendFax,
    ns__LogFuncType__ScanAndSend,
    ns__LogFuncType__ScanAndUload,
    ns__LogFuncType__DloadAndPrint
};

struct ns__LogItem
{
    char*                                  __item                   0;
    @enum ns__LogItemType                  attr                     1;
};

struct ns__JobLog
{
    @enum ns__LogFuncType                  func                     1;
    int                                    __sizeLogItem;
    struct ns__LogItem*                    LogItem                  1;
};

struct ns__JobLogs
{
    int                                    __sizeJobLog;
    struct ns__JobLog*                     JobLog                   1;
};

struct ns__EvLastUserLog
{
    int                                    ErrorInfo                1;
    struct ns__EventUserId*                UserId                   0;
    char*                                  LoginTime                1;
    char*                                  LogoutTime               1;
    struct ns__JobLogs*                    JobLogs                  0;
};


/* EV-xxx */
struct ns__EvPjlResult
{
    int                                    ErrorInfo                1;
    char*                                  Response                 1;
};

/* EV-603 RxFaxList */
enum ns__CallerType {
    ns__CallerType__Fax
};

struct ns__Caller
{
    char*                                  __item                   0;
    @enum ns__CallerType*                  type                     0;
};
struct ns__RxFaxItem
{
    @int                                   dataId                   1;
    struct ns__Caller*                     Caller                   0;
    char*                                  Date                     0;
};

struct ns__RxFaxItems
{
    int                                    __sizeRxFaxItem;
    struct ns__RxFaxItem*                  RxFaxItem                1;
};

struct ns__EvRxFaxList 
{
    int                                    ErrorInfo                1;
    struct ns__RxFaxItems*                 RxFaxItems               0;
};

/* EV-620 DbDeleteDone */

struct ns__EvDbDeleteDone
{
    int                                    ErrorInfo                1;
};

struct ns__Event
{
    int                                    __union_EventType;
    union _ns__union_EventType
    {
        struct ns__EvLaunchReqReceived     LaunchReqReceived        1; /* 201 */
        struct ns__EvLockDone              LockDone                 1; /* 401 */
        struct ns__EvUserInput             UserInput                1; /* 511 */
        struct ns__EvDbReadDone            DbReadDone               1; /* 601 */
        struct ns__EvRxFaxList             RxFaxList                1; /* 603 */
        struct ns__EvDbUpdateDone          DbUpdateDone             1; /* 610 */
        struct ns__EvDbDeleteDone          DbDeleteDone             1; /* 620 */
        struct ns__EvJobReceived           JobReceived              1; /* 731 */
        struct ns__EvJobStarted            JobStarted               1; /* 732 */
        struct ns__EvJobOnePagePrinted     JobOnePagePrinted        1; /* 733 */
        struct ns__EvJobDataCommDone       JobDataCommDone          1; /* 734 */
        struct ns__EvUlImgIdentified       UlImgIdentified          1; /* 735 */
        struct ns__EvDlImgIdentified       DlImgIdentified          1; /* 736 */
        struct ns__EvJobOneFileUloaded     JobOneFileUloaded        1; /* 737 */
        struct ns__EvJobOneFileDloaded     JobOneFileDloaded        1; /* 739 */
        struct ns__EvDeviceStatusChanged   DeviceStatusChanged      1; /* xxx */
        struct ns__EvJobStatusChanged      JobStatusChanged         1; /* xxx */
        struct ns__EvJobDone               JobDone                  1; /* 740 */
        struct ns__EvNotificationAborted   NotificationAborted      1; /* xxx */
        struct ns__EvSessionClosed         SessionClosed            1; /* 800 */
        struct ns__EvWaitTimeOut           WaitTimeOut              1; /* 810 */
        struct ns__EvLastUserLog           LastUserLog              1; /* XXX */
        struct ns__EvPjlResult             PjlResult                1;
        /* struct ns__EvIojobStatusUpdated IojobStatusUpdated       1; */
    }union_EventType;
};

struct ns__EvCommand
{
    @char*                                 version                1;
    struct  ns__Event                      __Event                1;
};


// パーサ用
int ns__SerioCommand(struct ns__CommandType __CommandType, int *result);

// ビルダ用
int ns__EventCommand(struct ns__EvCommand *__Event, int *result);

