
#include "SliceStreamPaths.H"

#include "OSspecific.H" // isDir


void Foam::SliceStreamPaths::checkFiles()
{
    if (!filesChecked_)
    {
        dataPresent_ = isDir(dataPathname_);
        meshPresent_ = isDir(meshPathname_);
        filesChecked_ = true;
    }
}


void Foam::SliceStreamPaths::setPathName(const Foam::fileName& pathname)
{
    pathname_ = pathname;
}


const Foam::fileName& Foam::SliceStreamPaths::getPathName() const
{
    return pathname_;
}


Foam::fileName Foam::SliceStreamPaths::meshPathname(const Foam::fileName& path)
{
    return path / meshPathname_;
}


Foam::fileName Foam::SliceStreamPaths::dataPathname(const Foam::fileName& path)
{
    return path / dataPathname_;
}


bool Foam::SliceStreamPaths::dataPresent()
{
    checkFiles();
    return dataPresent_;
}


bool Foam::SliceStreamPaths::meshPresent()
{
    checkFiles();
    return meshPresent_;
}

