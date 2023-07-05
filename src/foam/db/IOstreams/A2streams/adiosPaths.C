
#include "adiosPaths.H"

#include "OSspecific.H" // isDir


void Foam::adiosPaths::checkFiles()
{
    if (!filesChecked_)
    {
        dataPresent_ = isDir(dataPathname_);
        meshPresent_ = isDir(meshPathname_);
        filesChecked_ = true;
    }
}


void Foam::adiosPaths::setPathName(const Foam::fileName& pathname)
{
    pathname_ = pathname;
}


const Foam::fileName& Foam::adiosPaths::getPathName() const
{
    return pathname_;
}


Foam::fileName Foam::adiosPaths::meshPathname(const Foam::fileName& path)
{
    return path / meshPathname_;
}


Foam::fileName Foam::adiosPaths::dataPathname(const Foam::fileName& path)
{
    return path / dataPathname_;
}


bool Foam::adiosPaths::dataPresent()
{
    checkFiles();
    return dataPresent_;
}


bool Foam::adiosPaths::meshPresent()
{
    checkFiles();
    return meshPresent_;
}

