FileObject::FileObject()
{
  _buffer.reserve(10485760); // 10MB buffer
}

void FileObject::begin_receive_file(const QString &filename)
{
  _filename = filename;
  return;
}
