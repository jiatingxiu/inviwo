#ifndef IVW_VOLUMEWRITER_H
#define IVW_VOLUMEWRITER_H

#include "inviwo/core/io/datawriter.h"
#include "inviwo/core/metadata/metadatamap.h"

namespace inviwo {

class WriterSettings : public IvwSerializable {            
public:
    WriterSettings() {}    
    WriterSettings(std::string rawFile, ivec3 resolution=ivec3(0), std::string format="UCHAR");
    ~WriterSettings() {}

    //serialization
    virtual void serialize(IvwSerializer& s) const {
        s.serialize("RawFileAbsolutePath", rawFileAbsolutePath_);
        s.serialize("DataFormat", dataFormat_);
        s.serialize("Dimensions", dimensions_);        
    }

    //de-serialization
    virtual void deserialize(IvwDeserializer& d) {
        d.deserialize("RawFileAbsolutePath", rawFileAbsolutePath_);
        d.deserialize("DataFormat", dataFormat_);
        d.deserialize("Dimensions", dimensions_);        
    }

    //member varialbles
    std::string rawFileAbsolutePath_;
    uvec3 dimensions_;
    std::string dataFormat_;
    const void* texels_;
};

class IVW_CORE_API VolumeWriter : public DataWriter {

public:
    VolumeWriter();
    virtual ~VolumeWriter() {}

    virtual void writeData()=0;
};

} // namespace

#endif // IVW_VOLUMEWRITER_H
