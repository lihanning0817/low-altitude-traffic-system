# 本地环境配置脚本
# 用于快速设置本地开发环境

Write-Host "================================" -ForegroundColor Cyan
Write-Host "低空交通系统 - 本地环境配置向导" -ForegroundColor Cyan
Write-Host "================================" -ForegroundColor Cyan
Write-Host ""

# 检查 MySQL 服务
Write-Host "[1/6] 检查 MySQL 服务状态..." -ForegroundColor Yellow
$mysqlService = Get-Service -Name "MySQL57" -ErrorAction SilentlyContinue
if ($mysqlService) {
    if ($mysqlService.Status -eq "Running") {
        Write-Host "✅ MySQL 服务正在运行" -ForegroundColor Green
    } else {
        Write-Host "⚠️  MySQL 服务未运行，尝试启动..." -ForegroundColor Yellow
        Start-Service -Name "MySQL57"
        Write-Host "✅ MySQL 服务已启动" -ForegroundColor Green
    }
} else {
    Write-Host "❌ 未找到 MySQL57 服务，请手动检查 MySQL 安装" -ForegroundColor Red
}

# 获取 MySQL 密码
Write-Host ""
Write-Host "[2/6] 配置数据库连接..." -ForegroundColor Yellow
$mysqlPassword = Read-Host "请输入 MySQL root 密码" -AsSecureString
$BSTR = [System.Runtime.InteropServices.Marshal]::SecureStringToBSTR($mysqlPassword)
$mysqlPasswordPlain = [System.Runtime.InteropServices.Marshal]::PtrToStringAuto($BSTR)

# 更新本地配置文件
Write-Host ""
Write-Host "[3/6] 更新本地配置文件..." -ForegroundColor Yellow
$configPath = "$PSScriptRoot\..\backend\config\server.local.json"
$config = Get-Content $configPath -Raw | ConvertFrom-Json
$config.database.password = $mysqlPasswordPlain
$config | ConvertTo-Json -Depth 10 | Set-Content $configPath
Write-Host "✅ 配置文件已更新: $configPath" -ForegroundColor Green

# 测试数据库连接
Write-Host ""
Write-Host "[4/6] 测试数据库连接..." -ForegroundColor Yellow
$testConnection = "mysql -u root -p$mysqlPasswordPlain -e 'SELECT 1;' 2>&1"
$result = Invoke-Expression $testConnection
if ($LASTEXITCODE -eq 0) {
    Write-Host "✅ 数据库连接成功" -ForegroundColor Green

    # 初始化数据库
    Write-Host ""
    Write-Host "[5/6] 初始化数据库..." -ForegroundColor Yellow
    $initDbCmd = "mysql -u root -p$mysqlPasswordPlain < $PSScriptRoot\init-database.sql 2>&1"
    $dbResult = Invoke-Expression $initDbCmd
    if ($LASTEXITCODE -eq 0) {
        Write-Host "✅ 数据库初始化成功" -ForegroundColor Green
    } else {
        Write-Host "⚠️  数据库初始化可能失败，请检查输出" -ForegroundColor Yellow
        Write-Host $dbResult
    }
} else {
    Write-Host "❌ 数据库连接失败，请检查密码是否正确" -ForegroundColor Red
    Write-Host $result
    exit 1
}

# 检查前端依赖
Write-Host ""
Write-Host "[6/6] 检查前端依赖..." -ForegroundColor Yellow
if (Test-Path "$PSScriptRoot\..\node_modules") {
    Write-Host "✅ 前端依赖已安装" -ForegroundColor Green
} else {
    Write-Host "⚠️  前端依赖未安装，正在安装..." -ForegroundColor Yellow
    Set-Location "$PSScriptRoot\.."
    npm install
    Write-Host "✅ 前端依赖安装完成" -ForegroundColor Green
}

# 完成
Write-Host ""
Write-Host "================================" -ForegroundColor Cyan
Write-Host "✅ 本地环境配置完成！" -ForegroundColor Green
Write-Host "================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "下一步操作：" -ForegroundColor Yellow
Write-Host "1. 在 VS Code 中按 F5 选择 '🚀 启动完整应用 (前端+后端)'" -ForegroundColor White
Write-Host "2. 或使用终端命令：" -ForegroundColor White
Write-Host "   - 前端: npm run serve" -ForegroundColor Gray
Write-Host "   - 后端: .\backend\Release\low_altitude_traffic_system_backend.exe .\backend\config\server.local.json" -ForegroundColor Gray
Write-Host ""
Write-Host "访问地址：" -ForegroundColor Yellow
Write-Host "- 前端: http://localhost:8080" -ForegroundColor White
Write-Host "- 后端: http://localhost:8081" -ForegroundColor White
Write-Host ""
