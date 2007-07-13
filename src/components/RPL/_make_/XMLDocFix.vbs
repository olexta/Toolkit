'*******************************************************************************
' Project:     Robust Persistence Layer
'
' Module:      XMLDocFix.vbs
'     
' Content:     This VBScript provide ability to fix XML file with documentation
'
' Author:      Alexey Tkachuk
' Copyright:   Copyright © 2006-2007 Alexey Tkachuk
'              All Rights Reserved
'
' There is one "feature" in MC++ XMLDoc compiler: all XML documentation must be
' placed in header files. But this is not always useful, so documentation place
' in source files. There are no problems for functions in result XML file, but
' for properties, coments links to property accessor and not to own property.
' This script find all ocuurences of that comments by regular expression pattern:
' "M:(.+)(?:get_|set_)([\w\d_]+).*" and change it to "P:namespace.obj.prop".
'
' To use this script pass XML file path as parameter. All non existing files
' will be ignored
'
'*******************************************************************************

Option Explicit

Private Sub XMLDocFix(file)
    Dim reg 'As RegExp
    Dim fso 'As Scripting.FileSystemObject
    Dim ts 'As TextStream
    Dim s 'As String
   
    
    On Error Resume Next

    'create file system object
    Set fso = CreateObject("Scripting.FileSystemObject")
    
    'open specified file
    Set ts = fso.OpenTextFile(file, 1, False, -2) 'ForReading, TristateUseDefault
    
    'if file was opened successful (ignore non existing files)
    If (Err.Number = 0) Then
        'now all errors must be raised
        On Error GoTo 0
        
        'create a regular expression
        Set reg = CreateObject("VBScript.RegExp")
        'set regular expression options
        reg.Pattern = """M:(.+)(?:get_|set_)([\w\d_]+).*"""
        reg.IgnoreCase = True
        reg.Global = True
        'replace all founded matches by specified expression
        s = reg.Replace(ts.ReadAll, """P:$1$2""")
        Set reg = Nothing
        
        'and save results in file
        ts.Close
        Set ts = fso.CreateTextFile(file, True)
        Call ts.Write(s)
        ts.Close
        
        Set ts = Nothing
    End If
    Set fso = Nothing
End Sub


'---------------------------------
' MAIN CODE
'---------------------------------
If (Wscript.Arguments.Count = 1) Then
    'fix XML documentation file
     XMLDocFix( Wscript.Arguments(0) )     
Else                                           
    'invalid attributes
     MsgBox "Invalid script attributes!", vbExclamation, "Attention!"
End If
