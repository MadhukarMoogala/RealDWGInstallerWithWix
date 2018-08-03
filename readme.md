# RealDWG Installer using Wix Toolset
### Steps to build the sample

 - [Get Wix Toolset](http://wixtoolset.org/releases/v3.11.1/stable)
 - [Get Wix Toolset Extension for Visual Studio 2017](https://marketplace.visualstudio.com/vsgallery/2eb3402e-ea6d-4dcd-8340-c88435e54ea9)
 - [Get Visual Studio 2017](https://imagine.microsoft.com/en-us/Catalog/Product/530)
 - [Get RealDWG license](https://www.autodesk.com/developer-network/platform-technologies/realdwg)
 - Create a Enviroment Variable `REALDWG2019:C:\Program Files\Autodesk\RealDWG 2019`
 - Check for VC\MFC Redistributes  `CommonProgramFiles(x86)        C:\Program Files (x86)\Common Files\Merge Modules` and set  Environment Variable if not already exist  `CommonProgramFiles(x86): C:\Program Files (x86)\Common Files`
 - `git clone https://github.com/MadhukarMoogala/RealDWGInstallerWithWix.git`
 - `cd RealDWGInstallerWithWix`
 - `devenv RealDWGInstaller/RealDWGInstaller.sln`
 - `Build`