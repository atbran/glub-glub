param(
    [string]$ProcName = "Glub-Glub",
    [string]$OutDir = "$env:TEMP\glub_frames",
    [int]$Frames = 80,
    [int]$IntervalMs = 110
)

Add-Type -AssemblyName System.Drawing
Add-Type @"
using System;
using System.Text;
using System.Runtime.InteropServices;
public class WinCap {
  public delegate bool EnumProc(IntPtr h, IntPtr l);
  [DllImport("user32.dll")] public static extern bool EnumWindows(EnumProc cb, IntPtr l);
  [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr h, out uint pid);
  [DllImport("user32.dll")] public static extern int GetWindowText(IntPtr h, StringBuilder s, int n);
  [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr h);
  [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr h, out RECT r);
  [DllImport("user32.dll")] public static extern bool SetProcessDPIAware();
  [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h);
  [DllImport("user32.dll")] public static extern bool MoveWindow(IntPtr h, int x, int y, int w, int hh, bool r);
  public struct RECT { public int Left, Top, Right, Bottom; }

  public static IntPtr FindMain(uint pid, string contains, out int w, out int hgt)
  {
    IntPtr best = IntPtr.Zero; long bestArea = 0; int bw = 0; int bh = 0;
    EnumWindows((h, l) =>
    {
      uint wpid; GetWindowThreadProcessId(h, out wpid);
      if (wpid != pid || !IsWindowVisible(h)) return true;
      var t = new StringBuilder(256); GetWindowText(h, t, 256);
      if (!t.ToString().Contains(contains)) return true;
      var r = new RECT(); GetWindowRect(h, out r);
      long area = (long)(r.Right - r.Left) * (r.Bottom - r.Top);
      if (area > bestArea) { bestArea = area; best = h; bw = r.Right - r.Left; bh = r.Bottom - r.Top; }
      return true;
    }, IntPtr.Zero);
    w = bw; hgt = bh;
    return best;
  }
}
"@
[WinCap]::SetProcessDPIAware() | Out-Null

New-Item -ItemType Directory -Force -Path $OutDir | Out-Null
Get-ChildItem $OutDir -Filter *.png -ErrorAction SilentlyContinue | Remove-Item -Force

$procs = @(Get-Process -Name $ProcName -ErrorAction SilentlyContinue | Where-Object { $_.MainWindowHandle -ne 0 })
if ($procs.Count -eq 0) { throw "no process named $ProcName" }

$handle = [IntPtr]::Zero
$wd = 0; $ht = 0
foreach ($p in $procs)
{
    $h2 = [WinCap]::FindMain([uint32]$p.Id, $ProcName, [ref]$wd, [ref]$ht)
    if ($h2 -ne [IntPtr]::Zero) { $handle = $h2; break }
}
if ($handle -eq [IntPtr]::Zero) { throw "no matching visible window found" }

[WinCap]::MoveWindow($handle, 60, 60, 500, 540, $true) | Out-Null
Start-Sleep -Milliseconds 400
[WinCap]::SetForegroundWindow($handle) | Out-Null
Start-Sleep -Milliseconds 400

$r = New-Object WinCap+RECT
[WinCap]::GetWindowRect($handle, [ref]$r) | Out-Null
$wd = $r.Right - $r.Left
$ht = $r.Bottom - $r.Top
if ($wd -le 0 -or $ht -le 0) { throw "bad window rect ${wd}x${ht}" }

for ($i = 0; $i -lt $Frames; $i++)
{
    $bmp = New-Object System.Drawing.Bitmap $wd, $ht
    $g = [System.Drawing.Graphics]::FromImage($bmp)
    $g.CopyFromScreen($r.Left, $r.Top, 0, 0, (New-Object System.Drawing.Size($wd, $ht)))
    $g.Dispose()
    $bmp.Save((Join-Path $OutDir ("f{0:D3}.png" -f $i)), [System.Drawing.Imaging.ImageFormat]::Png)
    $bmp.Dispose()
    Start-Sleep -Milliseconds $IntervalMs
}

Write-Output "captured $Frames frames ($wd x $ht) to $OutDir"
