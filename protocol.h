#ifndef PROTOCOL
#define PROTOCOL

namespace HandShake {
  const quint32 RequestChangesList = 0;
  const quint32 RequestDeletedList = 1;
  const quint32 RequestModifiedList = 2;
  const quint32 SendingChangesList = 3;
  const quint32 SendingDeletedList = 4;
  const quint32 SendingModifiedList = 5;

  const quint32 RequestFile = 6;
  const quint32 SendingFile = 7;

  const quint32 DeleteFiles = 8;

  const quint32 SetDirectory = 9;
  const quint32 Reset = 10;
  const quint32 Acknowledge = 11;

  const char* strings[12] = {"RequestChangesList",
			     "RequestDeletedList",
			     "RequestModifiedList",
			     "SendingChangesList",
			     "SendingDeletedList",
			     "SendingModifiedList",
			     "RequestFile",
			     "SendingFile",
			     "DeleteFiles",
			     "SetDirectory",
			     "Reset",
			     "Acknowledge"};
};

#endif
