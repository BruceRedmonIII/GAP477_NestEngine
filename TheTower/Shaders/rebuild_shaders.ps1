$shaders = Get-ChildItem $PSScriptRoot -Filter "*.glsl"
$glslangValidator = $Env:VK_SDK_PATH + "\Bin\glslangValidator.exe"

foreach ($shader in $shaders)
{
	$glslPath = $shader.FullName
	$spvPath = $shader.FullName.replace('.glsl', '.spv')
	Start-Process -NoNewWindow -FilePath $glslangValidator -ArgumentList "-V -o $spvPath $glslPath"
}