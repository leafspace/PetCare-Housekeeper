<%@ LANGUAGE = VBScript %>
<HTML>
<TITLE>
Hello World
</TITLE>
<BODY>
<%
'以下循??出Hello World字符串,字体由小?大
for i=1 to 5
response.write "<font size=" & i & ">hello world</font><br>"
next 
%>
</BODY>
</HTML>