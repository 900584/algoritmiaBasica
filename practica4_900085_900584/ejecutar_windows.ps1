$ErrorActionPreference = "Stop"

Write-Host "[1/6] Compilando..."
if (Test-Path ".\formarEquipos.exe") { Remove-Item ".\formarEquipos.exe" -Force }
if (Test-Path ".\verificarBruteforce.exe") { Remove-Item ".\verificarBruteforce.exe" -Force }

gcc -O2 -Wall -Wextra -std=c11 -o formarEquipos.exe src/formarEquipos.c
gcc -O2 -Wall -Wextra -std=c11 -o verificarBruteforce.exe src/verificar_bruteforce.c

Write-Host "[2/6] Ejecutando Branch&Bound..."
.\formarEquipos.exe .\pruebas\pruebas.txt .\resultados\resultados_pruebas.txt
.\formarEquipos.exe .\pruebas\experimentos.txt .\resultados\resultados_experimentos_bnb.txt

Write-Host "[3/6] Verificando con fuerza bruta (instancias pequenas)..."
.\verificarBruteforce.exe .\pruebas\pruebas.txt .\resultados\resultados_pruebas_bruteforce.txt

Write-Host "[4/6] Comparacion BnB vs fuerza bruta..."
$bnbVals = (Select-String -Path ".\resultados\resultados_pruebas.txt" -Pattern "^Valor_optimo=") `
    | ForEach-Object { $_.Line -replace "Valor_optimo=", "" }
$bfVals = (Select-String -Path ".\resultados\resultados_pruebas_bruteforce.txt" -Pattern "^Valor_optimo=") `
    | ForEach-Object { $_.Line -replace "Valor_optimo=", "" }

$bnbJoin = ($bnbVals -join ",")
$bfJoin  = ($bfVals  -join ",")

if ($bnbJoin -eq $bfJoin) {
    Write-Host "OK: Branch&Bound coincide con fuerza bruta en pruebas/pruebas.txt"
} else {
    Write-Host "ATENCION: discrepancia detectada entre Branch&Bound y fuerza bruta"
    Write-Host "  BnB: $bnbJoin"
    Write-Host "  BF : $bfJoin"
}

Write-Host "[5/6] Ejecutando solucion ILP (programacion lineal)..."
$pythonCmd = $null
foreach ($cmd in @("python", "python3")) {
    if (Get-Command $cmd -ErrorAction SilentlyContinue) { $pythonCmd = $cmd; break }
}
if ($null -eq $pythonCmd) {
    Write-Host "AVISO: Python no encontrado. Instala Python y ejecuta: pip install pulp"
} else {
    $pulpCheck = & $pythonCmd -c "import pulp" 2>&1
    if ($LASTEXITCODE -ne 0) {
        Write-Host "AVISO: pulp no instalado. Ejecuta: pip install pulp"
    } else {
        & $pythonCmd src/resolverLP.py pruebas/pruebas.txt resultados/resultados_pruebas_lp.txt
        & $pythonCmd src/resolverLP.py pruebas/experimentos.txt resultados/resultados_experimentos_lp.txt
        Write-Host "ILP ejecutado correctamente."

        Write-Host "[6/6] Comparacion BnB vs ILP..."
        $lpVals = (Select-String -Path ".\resultados\resultados_pruebas_lp.txt" -Pattern "^Valor_optimo=") `
            | ForEach-Object { $_.Line -replace "Valor_optimo=", "" }
        $lpJoin = ($lpVals -join ",")

        if ($bnbJoin -eq $lpJoin) {
            Write-Host "OK: Branch&Bound coincide con ILP en pruebas/pruebas.txt"
        } else {
            Write-Host "ATENCION: discrepancia entre Branch&Bound e ILP"
            Write-Host "  BnB: $bnbJoin"
            Write-Host "  ILP: $lpJoin"
        }
    }
}

Write-Host ""
Write-Host "Ficheros generados:"
Write-Host " - resultados/resultados_pruebas.txt"
Write-Host " - resultados/resultados_experimentos_bnb.txt"
Write-Host " - resultados/resultados_pruebas_bruteforce.txt"
Write-Host " - resultados/resultados_pruebas_lp.txt"
Write-Host " - resultados/resultados_experimentos_lp.txt"
