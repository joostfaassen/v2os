program Install;

uses
  Forms,
  Main in 'Main.pas' {Form1};

{$R *.RES}

begin
  Application.Initialize;
  Application.Title := 'V2_OS installer for Windows';
  Application.CreateForm(TForm1, Form1);
  Application.Run;
end.
