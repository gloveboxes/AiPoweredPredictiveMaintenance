# dotnet publish -r win-x64 -p:PublishSingleFile=true --self-contained true --assembly-name test -c Release -o Publish/win-x64
# dotnet publish -r win-arm64 -p:PublishSingleFile=true --self-contained true -c Release -o Publish/win-arm64
# dotnet publish -r linux-x64 -p:PublishSingleFile=true --self-contained true -c Release -o Publish/linux-x64
# dotnet publish -r linux-arm64 -p:PublishSingleFile=true --self-contained true -c Release -o Publish/linux-arm64
# dotnet publish -r osx.13-arm64 -p:PublishSingleFile=true --self-contained true -c Release -o Publish/osx.13-arm64
# dotnet publish -r osx.13-x64 -p:PublishSingleFile=true --self-contained true -c Release -o Publish/osx.13-x64

dotnet publish -r win-x64 /p:PublishSingleFile=true,assemblyname=hvac-win-x64 --self-contained true  -c Release -o prebuilt
dotnet publish -r win-arm64 /p:PublishSingleFile=true,assemblyname=hvac-win-arm64 --self-contained true  -c Release -o prebuilt

dotnet publish -r linux-x64 /p:PublishSingleFile=true,assemblyname=hvac-linux-x64 --self-contained true  -c Release -o prebuilt
dotnet publish -r linux-arm64 /p:PublishSingleFile=true,assemblyname=hvac-linux-arm64 --self-contained true  -c Release -o prebuilt

dotnet publish -r osx.13-arm64 /p:PublishSingleFile=true,assemblyname=hvac-osx.13-arm64 --self-contained true  -c Release -o prebuilt
dotnet publish -r osx.13-x64 /p:PublishSingleFile=true,assemblyname=hvac-osx.13-x64 --self-contained true  -c Release -o prebuilt

Remove-Item prebuilt/*.pdb

git update-index --chmod=+x .\prebuilt\hvac-linux-arm64
git update-index --chmod=+x .\prebuilt\hvac-linux-x64
git update-index --chmod=+x .\prebuilt\hvac-osx.13-arm64
git update-index --chmod=+x .\prebuilt\hvac-osx.13-x64

# dotnet publish -r win-x64 /p:Configuration=Release,PublishSingleFile=true,IncludeNativeLibrariesForSelfExtract=true,AssemblyName=appname --self-contained --output ./releases/windows/v1