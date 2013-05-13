unit Main;

interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms, Dialogs,
  StdCtrls, ImgList, ComCtrls, IniFiles;

type
  TMediaType = (
    Unknown,                // Format is unknown
    F5_1Pt2_512,            // 5.25", 1.2MB,  512 bytes/sector
    F3_1Pt44_512,           // 3.5",  1.44MB, 512 bytes/sector
    F3_2Pt88_512,           // 3.5",  2.88MB, 512 bytes/sector
    F3_20Pt8_512,           // 3.5",  20.8MB, 512 bytes/sector
    F3_720_512,             // 3.5",  720KB,  512 bytes/sector
    F5_360_512,             // 5.25", 360KB,  512 bytes/sector
    F5_320_512,             // 5.25", 320KB,  512 bytes/sector

    F5_320_1024,            // 5.25", 320KB,  1024 bytes/sector
    F5_180_512,             // 5.25", 180KB,  512 bytes/sector
    F5_160_512,             // 5.25", 160KB,  512 bytes/sector
    RemovableMedia,         // Removable media other than floppy
    FixedMedia              // Fixed hard disk media
  );

  PDiskGeometry = ^TDiskGeometry;
  TDiskGeometry = record
   Cylinders: Int64;
   MediaType: TMediaType;
   TracksPerCylinder: LongWord;
   SectorsPerTrack: LongWord;
   BytesPerSector: LongWord;
  end;

  TForm1 = class(TForm)
    Label1: TLabel;
    TreeView1: TTreeView;
    ImageList1: TImageList;
    OKBtn: TButton;
    CancelBtn: TButton;
    OpenDialog1: TOpenDialog;
    procedure FormCreate(Sender: TObject);
    procedure CancelBtnClick(Sender: TObject);
    procedure OKBtnClick(Sender: TObject);
    procedure FormDestroy(Sender: TObject);
  private
    FImage: string;
  end;

var
  Form1: TForm1;

resourcestring

sBytes = '%d bytes';
sKB = '%.2f KB';
sMB = '%.2f MB';
sGB = '%.2f GB';

sLogical = '%s: %s';

sPhysical = 'Disk %d: %s';

sUnknown = 'Unknown';
sActivePartition = 'Partition type %1:.2x: %2:s, active, "%0:s"';
sPartition = 'Partition type %1:.2x: %2:s, "%0:s"';

sError = 'V2_OS image file ''%s'' not found.';

const
  FSTypes: array [0..53] of record
    t: byte;
    n: string
  end = (
    (t: $01; n: 'FAT12'),
    (t: $04; n: 'FAT16 < 32 MB'),
    (t: $05; n: 'Extended'),
    (t: $06; n: 'FAT16 > 32 MB'),
    (t: $07; n: 'HPFS or NTFS'),
    (t: $0A; n: 'OS/2 Boot Manager'),
    (t: $0B; n: 'FAT32'),
    (t: $0C; n: 'LBA FAT32'),
    (t: $0E; n: 'LBA FAT16 > 32 MB'),
    (t: $0F; n: 'LBA extended'),
    (t: $11; n: 'hidden FAT12'),
    (t: $14; n: 'hidden FAT16 < 32 MB'),
    (t: $16; n: 'hidden FAT16 > 32 MB'),
    (t: $17; n: 'hidden HPFS or NTFS'),
    (t: $18; n: 'Zero-Volt Suspend'),
    (t: $1B; n: 'hidden FAT32'),
    (t: $1C; n: 'hidden LBA FAT32'),
    (t: $1E; n: 'hidden LBA VFAT'),
    (t: $3C; n: 'PartitionMagic recovery'),
    (t: $51; n: 'NOVELL'),
    (t: $55; n: 'EZ-Drive'),
    (t: $56; n: 'GoldenBow VFeature'),
    (t: $5C; n: 'Priam EDISK'),
    (t: $63; n: 'Mach'),
    (t: $64; n: 'Novell NetWare 286'),
    (t: $65; n: 'Novell NetWare (3.11)'),
    (t: $67; n: 'Novell'),
    (t: $68; n: 'Novell'),
    (t: $69; n: 'Novell'),
    (t: $6F; n: 'V2_OS RootFS'),
    (t: $75; n: 'PC/IX'),
    (t: $7E; n: 'F.I.X.'),
    (t: $80; n: 'Minix v1.1 - 1.4a'),
    (t: $81; n: 'Minix v1.4b+'),
    (t: $82; n: 'Linux Swap/Solaris'),
    (t: $83; n: 'Ext2fs/xiafs'),
    (t: $85; n: 'Linux EXT'),
    (t: $86; n: 'FAT16 volume/stripe set'),
    (t: $87; n: 'HPFS Fault-Tolerant mirrored or NTFS volume/stripe set'),
    (t: $99; n: 'Mylex EISA SCSI'),
    (t: $A5; n: 'FreeBSD, BSD/386'),
    (t: $A6; n: 'OpenBSD'),
    (t: $A9; n: 'NetBSD'),
    (t: $B6; n: 'Mirror set (master), FAT16'),
    (t: $B7; n: 'Mirror set (master), NTFS'),
    (t: $BE; n: 'Solaris boot'),
    (t: $C0; n: 'DR DOS/DR-DOS/Novell DOS secured partition/CTOS'),
    (t: $C6; n: 'Mirror set (slave), FAT16'),
    (t: $C7; n: 'Mirror set (slave), NTFS'),
    (t: $EB; n: 'BeOS BFS (BFS1)'),
    (t: $F2; n: 'DOS 3.3+ secondary'),
    (t: $FE; n: 'LANstep'),
    (t: $FE; n: 'IBM PS/2 Initial Microcode Load'),
    (t: $FF; n: 'Xenix bad block table'));

  Images: array [TMediaType] of integer =
    (1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 1);

implementation

{$R *.DFM}

type
  PRecord = ^TRecord;
  TRecord = record
    Path: string;
    Start: Int64;
    Partition: integer;
  end;

  TMBR = packed record
    Code: array [0..445] of byte;
    Partitions: array [0..3] of record
      Active: byte;
      StartCHS: array [0..2] of byte;
      FS: byte;
      EndCHS: array [0..2] of byte;
      StartSector: LongWord;
      Sectors: LongWord;
    end;
    Signature: word;
  end;

function FormatSize(x: Int64): string;
begin
  if x >= 1024 * 1024 * 1024 then
    Result := Format(sGB, [x / (1024 * 1024 * 1024)])
  else if x >= 1024 * 1024 then
    Result := Format(sMB, [x / (1024 * 1024)])
  else if x >= 1024 then
    Result := Format(sKB, [x / 1024])
  else
    Result := Format(sBytes, [x]);
end;

procedure TForm1.FormCreate(Sender: TObject);
var
  i, f, x: integer;
  c: cardinal;
  g: TDiskGeometry;
  n, n2: TTreeNode;
  p, s, path: string;
  imgsize, size: Int64;
  r: PRecord;
  MBR: TMBR;

  function ComputeSize: Int64;
  begin
    Result := g.Cylinders * g.TracksPerCylinder * g.SectorsPerTrack *
      g.BytesPerSector;
  end;

begin
  f := -1;
  try
    if ParamCount = 0 then
      FImage := ExtractFilePath(paramstr(0)) + '\V2_OS.img'
    else
      FImage := ParamStr(1);
    repeat
      f := FileOpen(PChar(FImage), fmOpenRead or fmShareDenyNone);
      if f = -1 then
      begin
        if OpenDialog1.Execute then
          FImage := OpenDialog1.FileName
        else
          halt(1);
      end;
    until f <> -1;
    Int64Rec(imgsize).Lo := GetFileSize(f, @Int64Rec(imgsize).Hi);
    Win32Check(Int64Rec(imgsize).Lo <> $ffffffff);
    FileClose(f);
    imgsize := (imgsize + 511) shr 9 shl 9;
    for i := 0 to 25 do
    begin
      path := '\\.\' + chr(ord('A') + i) + ':';
      f := FileOpen(path, fmOpenRead or fmShareDenyNone);
      if f <> -1 then
      begin
        if DeviceIOControl(f, $70000, nil, 0, @g, sizeof(g), c, nil) then
        begin
          size := ComputeSize;
          if (size >= imgsize) and (g.MediaType <> FixedMedia) and
            (g.BytesPerSector = 512) then
          begin
            New(r);
            r^.Path := path;
            r^.Start := 0;
            r^.Partition := -1;
            n := TreeView1.Items.AddObject(nil, Format(sLogical, [chr(ord('A') + i),
               FormatSize(size)]), r);
            n.ImageIndex := Images[g.MediaType];
            n.SelectedIndex := n.ImageIndex;
            Win32Check(FileSeek(f, Int64(0), 0) <> -1);
            Win32Check(FileRead(f, MBR, 512) = 512);
          end;
        end;
        FileClose(f);
      end;
    end;
    i := 0;
    repeat
      path := '\\.\PHYSICALDRIVE' + IntToStr(i);
      f := FileOpen(path, fmOpenRead or fmShareDenyNone);
      if f = -1 then break;
      Win32Check(DeviceIoControl(f, $70000, nil, 0, @g, sizeof(g), c, nil));
      size := ComputeSize;
      if g.BytesPerSector = 512 then
      begin
        n := TreeView1.Items.AddObject(nil, Format(sPhysical,
          [i, FormatSize(size)]), nil);
        n.ImageIndex := 1;
        n.SelectedIndex := 1;
        Win32Check(FileSeek(f, Int64(0), 0) <> -1);
        Win32Check(FileRead(f, MBR, 512) = 512);
        for c := 0 to 3 do
          with MBR.Partitions[c] do
          begin
            if (FS <> 0) and (Int64(Sectors) * 512 >= imgsize) then
            begin
              for x := 0 to 53 do
                if FSTypes[x].t >= FS then
                begin
                  if FSTypes[x].t = FS then
                    s := FSTypes[x].n
                  else
                    s := 'sUnknown';
                  break;
                end;
              New(r);
              r^.Path := path;
              r^.Start := Int64(StartSector) * 512;
              r^.Partition := c;
              if Active = 0 then p := sPartition else p := sActivePartition;
              n2 := TreeView1.Items.AddChildObject(n,
                 Format(p, [s, FS, FormatSize(Int64(Sectors) * 512)]), r);
              n2.ImageIndex := 2;
              n2.SelectedIndex := 2;
            end;
          end;
      end;
      FileClose(f);
      f := -1;
      inc(i);
    until false;
  finally
    if f <> -1 then FileClose(f);
  end;
end;

procedure TForm1.CancelBtnClick(Sender: TObject);
begin
  Close;
end;

procedure TForm1.OKBtnClick(Sender: TObject);
var
  s, d, count: integer;
  Buffer: pointer;
  MBR: TMBR;
begin
  if not Assigned(TreeView1.Selected) then exit;
  if not Assigned(TreeView1.Selected.Data) then exit;
  with PRecord(TreeView1.Selected.Data)^ do
  begin
    GetMem(Buffer, 32768);
    s := -1;
    d := -1;
    try
      s := FileOpen(FImage, fmOpenRead or fmShareDenyNone);
      Win32Check(s <> -1);
      Win32Check(FileSeek(s, 0, 0) <> -1);
      d := FileOpen(Path, fmOpenWrite or fmShareDenyNone);
      Win32Check(d <> -1);
      Win32Check(FileSeek(d, Start, 0) <> -1);
      repeat
        count := FileRead(s, Buffer^, 32768);
        if count = 0 then break;
        Win32Check(count <> -1);
        count := (count + 511) shr 9 shl 9;
        Win32Check(FileWrite(d, Buffer^, count) = count);
      until false;
      if FileExists('C:\boot.ini') and (Partition >= 0) then
      begin
        Win32Check(FileSeek(d, 0, 0) <> -1);
        Win32Check(FileRead(d, MBR, 512) = 512);
        MBR.Partitions[Partition].FS := $6f;
        Win32Check(FileSeek(d, 0, 0) <> -1);
        Win32Check(FileWrite(d, MBR, 512) = 512);
        FileClose(d);
        d := FileCreate('C:\V2_OS.boot');
        Win32Check(d <> -1);
        Win32Check(FileSeek(s, 0, 0) <> -1);
        Win32Check(FileSeek(d, 0, 0) <> -1);
        Win32Check(FileRead(s, Buffer^, 512) = 512);
        Win32Check(FileWrite(d, Buffer^, 512) = 512);
        with TIniFile.Create('C:\boot.ini') do
        try
          WriteString('operating systems', 'C:\V2_OS.boot', '"V2_OS"');
        finally
          Free;
        end;
      end;
    finally
      if s <> -1 then FileClose(s);
      if d <> -1 then FileClose(d);
      FreeMem(Buffer);
    end;
    Close;
  end;
end;

procedure TForm1.FormDestroy(Sender: TObject);
var i: integer;
begin
  for i := 0 to TreeView1.Items.Count - 1 do
    if Assigned(TreeView1.Items[i].Data) then
      FreeMem(TreeView1.Items[i].Data);
end;

end.
