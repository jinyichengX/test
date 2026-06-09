<#
.SYNOPSIS
    ESDBox_IPGUI HTML to DOCX converter via Word COM
.PARAMETER HtmlPath
    Path to HTML source file
.PARAMETER DocxPath
    Output DOCX file path
.EXAMPLE
    powershell -File convert_to_docx.ps1 -HtmlPath "output/manual.html" -DocxPath "output/manual.docx"
#>

param(
    [string]$HtmlPath = "$PSScriptRoot/output/ESDBox_IPGUI_manual.html",
    [string]$DocxPath = "$PSScriptRoot/output/ESDBox_IPGUI_manual.docx"
)

# Check if HTML exists
if (-not (Test-Path $HtmlPath)) {
    $msg = "HTML file not found: $HtmlPath"
    Write-Error $msg
    exit 1
}

# Start Word
Write-Host "[WordCOM] Starting Microsoft Word..."
try {
    $word = New-Object -ComObject Word.Application
    $word.Visible = $false
    $word.DisplayAlerts = 0
} catch {
    Write-Error "Cannot start Microsoft Word. Please verify Office is installed."
    exit 1
}

try {
    Write-Host "[WordCOM] Opening HTML file: $HtmlPath"
    $doc = $word.Documents.Open($HtmlPath, $false, $true)

    # Page setup: 185mm x 260mm
    $section = $doc.Sections.Item(1)
    $section.PageSetup.PageWidth    = 504
    $section.PageSetup.PageHeight   = 741
    $section.PageSetup.LeftMargin   = 63
    $section.PageSetup.RightMargin  = 57
    $section.PageSetup.TopMargin    = 63
    $section.PageSetup.BottomMargin = 57

    # Update fields (if any)
    $doc.Fields.Update()

    # Create output directory
    $outDir = Split-Path $DocxPath -Parent
    if (-not (Test-Path $outDir)) {
        New-Item -ItemType Directory -Force -Path $outDir | Out-Null
    }

    # Remove existing file
    if (Test-Path $DocxPath) {
        Remove-Item $DocxPath -Force
    }

    # Save as DOCX (wdFormatDocumentDefault = 16)
    Write-Host "[WordCOM] Saving DOCX to: $DocxPath"
    $doc.SaveAs2([ref]$DocxPath, [ref]16)

    $sizeKB = [math]::Round((Get-Item $DocxPath).Length / 1KB, 1)
    Write-Host "[OK] DOCX saved: $DocxPath ($sizeKB KB)"

} catch {
    Write-Error "Word COM error: $_"
    exit 1
} finally {
    if ($doc) { $doc.Close($false) }
    $word.Quit()
    [System.Runtime.InteropServices.Marshal]::ReleaseComObject($word) | Out-Null
    Write-Host "[WordCOM] Word closed"
}
