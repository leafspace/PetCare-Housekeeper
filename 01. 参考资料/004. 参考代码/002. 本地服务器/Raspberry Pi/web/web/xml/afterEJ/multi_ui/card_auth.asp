<% @LANGUAGE = "PerlScript" %>

<%

'***** Initialize
curPath = Server.MapPath("./")
logFolder = "log"
logFile   = "cardid_auth_log.txt"
iniFile   = "setting.ini"

'***************** Read electrical message **********
Dim i, pos, key, val, attr
Dim c, str

RequestBin=Request.BinaryRead(Request.TotalBytes)
For i = 1 To Request.TotalBytes
	c = MidB(RequestBin,i,1)
   	If AscB(c) < 32 Or AscB(c) > 255 Then str = str & "." Else str = str & Chr(AscB(c))
Next

pos = InStr(str, "<KeyValueData>")
'attr = GetTagAttr(str, pos, "attr")		'#未実装 とりあえずは必要なし
key = GetTagData(str, pos, "Key")
val = GetTagData(str, pos, "Value")

'******* Read Setting file *********
Dim objFS, objText, iniText
Set objFS  = CreateObject("Scripting.FileSystemObject")
Set objText = objFS.OpenTextFile(curPath & "\" & iniFile)
iniText = objText.ReadAll
objText.Close

Set objFS  = Nothing

'******* Get settings *********
Dim count, PFPath, User(10), CardID(10)

'配列　0:Permission、1:Color
Dim Copy(2, 10)
Dim SendFax(2, 10)
Dim ScanSend(2, 10)
Dim DirectPrint(2, 10)
Dim PrintFax(2, 10)
Dim PcPrint(2, 10)
Dim PageLimit(10)
Dim PageMax(10)

PFPath=readSetting(iniText, "PrintFilePath=")
For count = 0 To 9
	User(count)=readSetting(iniText, "User" & count & "=")
	CardID(count)=readSetting(iniText, "CardID" & count & "=")
	Copy(0, count)=readSetting(iniText, "Copy_Permission" & count & "=")
	Copy(1, count)=readSetting(iniText, "Copy_Color" & count & "=")
	SendFax(0, count)=readSetting(iniText, "SendFax_Permission" & count & "=")
	SendFax(1, count)=readSetting(iniText, "SendFax_Color" & count & "=")
	ScanSend(0, count)=readSetting(iniText, "ScanAndSend_Permission" & count & "=")
	ScanSend(1, count)=readSetting(iniText, "ScanAndSend_Color" & count & "=")
	DirectPrint(0, count)=readSetting(iniText, "DirectPrint_Permission" & count & "=")
	DirectPrint(1, count)=readSetting(iniText, "DirectPrint_Color" & count & "=")
	PrintFax(0, count)=readSetting(iniText, "PrintFax_Permission" & count & "=")
	PrintFax(1, count)=readSetting(iniText, "PrintFax_Color" & count & "=")
	PcPrint(0, count)=readSetting(iniText, "PcPrint_Permission" & count & "=")
	PcPrint(1, count)=readSetting(iniText, "PcPrint_Color" & count & "=")
	PageLimit(count)=readSetting(iniText, "PCPrint_PageLimit" & count & "=")
	PageMax(count)=readSetting(iniText, "PCPrint_PageMax" & count & "=")
Next


' ***************** Check key and select authentication method (UserID or CardID) *********************
Dim userName
Dim id

'If key = "card" Then 'by CardID
	For id = 0 To 9 Step 1
		If val = CardID(id) Then
			userName = User(id)
			Exit For
		End If
	Next

	If id < 9 Then
		Response.Write("<?xml version=" & Chr(34) & "1.0" & Chr(34) & " encoding=" & Chr(34) & "UTF-8" & Chr(34) & "?>") & vbCrLf
		Response.Write("<SerioCommands version=" & Chr(34) & "1.0" & Chr(34) & ">") & vbCrLf
		Response.Write(" <ActivateLock>") & vbCrLf
		Response.Write("  <UserId>" & userName & "</UserId>") & vbCrLf
		Response.Write("  <LockInfo>") & vbCrLf
		Response.Write("   <Copy permitted=" & Chr(34) & Copy(0,id) & Chr(34) & " colorEnable="  & Chr(34) & Copy(1,id) & Chr(34) & "></Copy>") & vbCrLf
		Response.Write("   <SendFax permitted=" & Chr(34) & SendFax(0,id) & Chr(34) & " colorEnable="  & Chr(34) & SendFax(1,id) & Chr(34) & "></SendFax>") & vbCrLf 
		Response.Write("   <ScanAndSend permitted=" & Chr(34) & ScanSend(0,id) & Chr(34) & " colorEnable="  & Chr(34) & ScanSend(1,id) & Chr(34) & "></ScanAndSend>") & vbCrLf 
		Response.Write("   <DirectPrint permitted=" & Chr(34) & DirectPrint(0,id) & Chr(34) & " colorEnable="  & Chr(34) & DirectPrint(0,id) & Chr(34) & "></DirectPrint>") & vbCrLf 
		Response.Write("   <PrintFax permitted=" & Chr(34) & PrintFax(0,id) & Chr(34) & " colorEnable="  & Chr(34) & PrintFax(1,id) & Chr(34) & "></PrintFax>") & vbCrLf 
		Response.Write("   <PcPrint permitted=" & Chr(34) & PcPrint(0,id) & Chr(34) & " colorEnable="  & Chr(34) & PcPrint(0,id) & Chr(34) & "></PcPrint>") & vbCrLf 
		Response.Write("  </LockInfo>") & vbCrLf
		'Response.Write("  <JobFinAckUrl>./resultoflock.xml" & "?plimit=" & PageLimit(id) & "&amp;pmax=" & PageMax(id) & "</JobFinAckUrl>") & vbCrLf
		Response.Write("  <JobFinAckUrl>./resultoflock.xml</JobFinAckUrl>") & vbCrLf
		Response.Write(" </ActivateLock>") & vbCrLf
		Response.Write("</SerioCommands>")
	Else
		Server.Execute "./unregisterd_message.xml"
	End If


'*****************************************************************************
'							Function
'*****************************************************************************

'*****************************************************************************
'Function:		GetTagData()
'param: inText	検索対象の文字列
'param: pos		検索開始位置
'param: tag		検索するtag
'return			検索タグに設定された値
'
'note			電文からtagの値を読み出す
'*****************************************************************************
Function GetTagData(inText,pos,tag)
Dim spos
Dim size
Dim param
Dim sTag, eTag

sTag = "<" & tag & ">"
eTag = "</" & tag & ">"

if pos < 1 Then
	pos = 1
End If

spos = (InStr(pos, inText, sTag) + Len(sTag))
size = (InStr(pos, inText, eTag) - spos)

If size > 0 Then
	param = Mid(inText,spos,size)
End If
GetTagdata = param

End Function
'*****************************************************************************

'*****************************************************************************
'Function:		readSetting()
'param: text	設定Fileの文字列
'param: data	読み出したい設定パラメータ
'return			パラメータ
'
'note			設定ファイルからパラメータを読み出す
'*****************************************************************************
Function readSetting(text, param)
dim pos,spos,plen,data
pos = InStr(text,param)
If pos > 0 Then
	spos = pos + Len(param)						'Parameter検索開始位置
	plen = InStr(spos, text, vbCRLF) - spos
	If plen > 0 Then
		data = Mid(text,spos,plen)
	Else
		data = Mid(text,spos)
	End If
End If
readSetting = data
End Function

'******** log (for debug) ************
Dim logName
Dim objSrvFS

set objSrvFS=Server.CreateObject("Scripting.FileSystemObject")
If objSrvFs.FolderExists(curPath & "\" & logFolder) = false Then
	objSrvFS.CreateFolder(curPath & "\" & logFolder)
End If
set logName=objSrvFS.CreateTextFile(curPath & "\" & logFolder  & "\" & LogFile,true)

logName.WriteLine ("Date:" & Now())
logName.WriteLine ("ScriptPath:" & curPath)
logName.WriteLine ("Val:" & val)
logName.WriteLine ("Key:" & key)
logName.WriteLine ("PFPATH:" & PFPath)

logName.WriteLine ("LoginUserName:"& userName)
logName.WriteLine ("Copy:"& Copy(0, 1) & " " & Copy(1, 1))
logName.WriteLine ("Fax:"& SendFax(0, 1) & " " & SendFax(1, 1))
logName.WriteLine ("ScanSend:"& ScanSend(0, 1) & " " & ScanSend(1, 1))
logName.WriteLine ("DirectPrint:"& DirectPrint(0, 1) & " " & DirectPrint(1, 1))
logName.WriteLine ("PrintFax:"& PrintFax(0, 1) & " " & PrintFax(1, 1))
logName.WriteLine ("PcPrint:"& PcPrint(0, 1) & " " & PcPrint(1, 1))
logName.WriteLine ("PageLimit:"& PageLimit(1))
logName.WriteLine ("PageMax:"& PageMax(1))
logName.WriteLine ("[RCV DATA]")
logName.WriteLine ("TEXT START----->>>>")
logName.WriteLine (str)
logName.WriteLine ("<<<<-----TEXT END")
logName.WriteLine ("[SETTING FILE]")
logName.WriteLine ("TEXT START-----")
logName.WriteLine (iniText)
logName.WriteLine ("-----TEXT END")


logName.close



'Response.Redirect "filelist.asp"

%>