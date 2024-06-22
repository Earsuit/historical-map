#Requires -Version 7

param (
    [string]$poFile
)

Set-Content $poFile -NoNewline -Encoding UTF8  -Value `
           ( ((Get-Content $poFile -Encoding UTF8 | `
            Select-String -Pattern "POT-Creation-Date:" -NotMatch | `
            Select-String -Pattern "PO-Revision-Date:" -NotMatch) -join "`n") + "`n")