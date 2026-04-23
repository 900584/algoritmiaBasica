$ErrorActionPreference = "Stop"

Write-Host "[1/4] Compilando..."
if (Test-Path ".\formarEquipos.exe") { Remove-Item ".\formarEquipos.exe" -Force }
if (Test-Path ".\verificarBruteforce.exe") { Remove-Item ".\verificarBruteforce.exe" -Force }

gcc -O2 -Wall -Wextra -std=c11 -o formarEquipos.exe src/formarEquipos.c
gcc -O2 -Wall -Wextra -std=c11 -o verificarBruteforce.exe src/verificar_bruteforce.c

Write-Host "[2/4] Ejecutando Branch&Bound..."
.\formarEquipos.exe .\pruebas\pruebas.txt .\resultados\resultados_pruebas.txt
.\formarEquipos.exe .\pruebas\experimentos.txt .\resultados\resultados_experimentos_bnb.txt

Write-Host "[3/4] Verificando con fuerza bruta (instancias pequenas)..."
.\verificarBruteforce.exe .\pruebas\pruebas.txt .\resultados\resultados_pruebas_bruteforce.txt

Write-Host "[4/4] Comparacion basica de valores optimos..."
$bnbVals = (Select-String -Path ".\resultados\resultados_pruebas.txt" -Pattern "^Valor_optimo=") `
    | ForEach-Object { $_.Line -replace "Valor_optimo=", "" }
$bfVals = (Select-String -Path ".\resultados\resultados_pruebas_bruteforce.txt" -Pattern "^Valor_optimo=") `
    | ForEach-Object { $_.Line -replace "Valor_optimo=", "" }

$bnbJoin = ($bnbVals -join ",")
$bfJoin = ($bfVals -join ",")

if ($bnbJoin -eq $bfJoin) {
    Write-Host "OK: Branch&Bound coincide con fuerza bruta en pruebas/pruebas.txt"
} else {
    Write-Host "ATENCION: discrepancia detectada entre Branch&Bound y fuerza bruta"
    Write-Host "  BnB: $bnbJoin"
    Write-Host "  BF : $bfJoin"
}

Write-Host ""
Write-Host "Ficheros generados:"
Write-Host " - resultados/resultados_pruebas.txt"
Write-Host " - resultados/resultados_experimentos_bnb.txt"
Write-Host " - resultados/resultados_pruebas_bruteforce.txt"
