rem calling script should use this like this:
rem call get_upi.cmd
rem %UPI_EXE% list

rem get_upi version control at https://gitlab-ncsa.ubisoft.org/piciteam/get_upi
set UPI_VERSION=366
set UPI_DIR_ABSPATH=%userprofile%\upi\p\upi\%UPI_VERSION%
set UPI_EXE=%UPI_DIR_ABSPATH%\upi.exe
set SRC_URL=https://artifactory.ubisoft.org/generic/upi/upi/upi4-upi-%UPI_VERSION%.zip
set DST_ZIP_ABSPATH=%userprofile%\upi\temp\upi4-upi-%UPI_VERSION%.zip
if not exist %UPI_DIR_ABSPATH%\.meta\upi_install_done.flag (
	rd /s /q %UPI_DIR_ABSPATH%
	md %userprofile%\upi\temp
	powershell -NoLogo -Command "add-type 'using System.Net;using System.Security.Cryptography.X509Certificates;public class TrustAllCertsPolicy : ICertificatePolicy {public bool CheckValidationResult(ServicePoint srvPoint, X509Certificate certificate,WebRequest request, int certificateProblem) {return true;}}';[System.Net.ServicePointManager]::CertificatePolicy = New-Object TrustAllCertsPolicy;(new-object System.Net.WebClient).DownloadFile($env:SRC_URL, $env:DST_ZIP_ABSPATH)"
	md %UPI_DIR_ABSPATH%
	powershell -NoLogo Expand-Archive %DST_ZIP_ABSPATH% -DestinationPath %UPI_DIR_ABSPATH% -Force
	type nul > %UPI_DIR_ABSPATH%\.meta\upi_install_done.flag
)
