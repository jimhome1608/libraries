unit Unit1;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls, ExtCtrls;

type
  TForm1 = class(TForm)
    Timer1: TTimer;
    Memo1: TMemo;
    procedure Timer1Timer(Sender: TObject);
  private
    FHandle: THandle;
  public

   function TComPortOpen() : Boolean;
   function TComPortRead() : String;
  end;

var
  Form1: TForm1;

implementation

{$R *.dfm}

function TForm1.TComPortOpen() : Boolean;
var
PortDCB : TDCB;
Timeouts : COMMTIMEOUTS;
begin
Result := False;
FHandle := CreateFile(PChar('COM11'),
GENERIC_READ or GENERIC_WRITE,
0, nil, OPEN_EXISTING,
0, 0); //trying to open com port

if (FHandle = INVALID_HANDLE_VALUE) then
exit;//com port was nor oppened for some reasons

FillChar(PortDCB, sizeof(TDCB), #0);
PortDCB.DCBlength := sizeof(TDCB);
GetCommState(FHandle, PortDCB); //getting port state

PortDCB.BaudRate := 9600;
PortDCB.ByteSize := 8;
PortDCB.Parity := NOPARITY;
PortDCB.StopBits := ONESTOPBIT;
PortDCB.Flags := 4243;

if (not SetCommState(FHandle, PortDCB)) then
exit;

Timeouts.ReadIntervalTimeout := MAXDWORD;
Timeouts.ReadTotalTimeoutMultiplier := MAXDWORD;
Timeouts.ReadTotalTimeoutConstant := 32000; //read timeout
Timeouts.WriteTotalTimeoutMultiplier := MAXDWORD;
Timeouts.WriteTotalTimeoutConstant := 32000; //write timeout

if (not SetCommTimeouts(FHandle, Timeouts)) then
exit;

EscapeCommFunction(FHandle, 11);

Result := True;
end;

function TForm1.TComPortRead() : String;
var
dwRead : Cardinal;
m_pBuf: PChar;
begin
Result := '';
//reading from port
ReadFile(FHandle, m_pBuf^, 1024, dwRead, nil);
Result := String(m_pBuf);
SetLength(Result, dwRead);
end;

procedure TForm1.Timer1Timer(Sender: TObject);
begin
   Memo1.Lines.Add(TComPortRead());
end;

end.
