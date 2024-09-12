#ifndef CONNECTION_H
#define CONNECTION_H

#include "Concepts.h"
#include "ThreadSafeQueue.h"
#include "SpatialFunction.h"

template<SpaceTime SPACETIME> class AgentBase;
template<class T> class Writer;
template<class T> class SendableWriter;

template<class T>
class Reader {
    Writer<T> *writer;
    ThreadSafeQueue<SpatialFunction<T>> buffer;
public:
    typedef T::SpaceTime SpaceTime;

    Reader() : writer(nullptr) { }

    Reader(Writer<T> &writerToConnectTo) : writer(nullptr) {
        connectTo(writerToConnectTo);
    }

    Reader(const Reader<T> &doNotUse) { 
        throw(std::runtime_error("Don't copy Readers: copy constructor only implemented to allow capture in std::function pre C++23"));
    }

    Reader(Reader<T> &&other) : writer(other.writer), buffer(std::move(other.buffer)) {
        other.writer = nullptr;
        if(writer != nullptr) writer->connectTo(this);
    }

    ~Reader() {
        if(!isClosed()) writer->remoteClose();
    }

    void connectTo(Writer<T> &newWriter) {
        signalNewWriter(newWriter);
        newWriter.signalNewReader(*this);
    }

    void signalNewWriter(Writer<T> &newWriter) {
        if(writer != nullptr) writer->remoteClose();
        writer = &newWriter;
    }

    // called by other end when they have closed the connection
    void remoteClose() {
        writer = nullptr;
    }

    // called by target to close the connection
    void close() {
        if(writer != nullptr) {
            writer->remoteClose();
            writer = nullptr;
        }
    }


    const SpaceTime &position() const {
        return (buffer.empty() ?
            (writer != nullptr ? writer->position() : SpaceTime::TOP) 
            : buffer.front().position());
    }

    template<class LAMBDA>
    inline void setCallback(LAMBDA &&lambda) {
        writer->setCallback(std::forward<LAMBDA>(lambda));
    }

    template<std::convertible_to<std::function<void(T &)>> LAMBDA>
    inline void push(const SpaceTime &position, LAMBDA &&function) { buffer.emplace(position, std::forward<LAMBDA>(function)); }

    bool executeNext(T &target) {
        if(buffer.empty()) return false;
        buffer.front()(target);
        buffer.pop();
        return true;
    }


    bool isClosed() const { return writer == nullptr; }

    Reader<T> &operator =(Reader<T> &&other) {
        buffer = std::move(other.buffer);
        if(other.writer != nullptr) this->connectTo(other.writer); else close();
        return *this;
    }
};


template<class T>
class Writer {
protected:
    typedef T::SpaceTime SpaceTime;

    Reader<T> *reader;
    AgentBase<typename T::SpaceTime> *source;

    friend class Reader<T>;

    void signalNewReader(Reader<T> &newReader) {
        if(reader != nullptr) reader->remoteClose();
        reader = &newReader;
    }

    // called by other end when they have closed the connection
    inline void remoteClose() { reader = nullptr; }

    template<class LAMBDA>
    inline void setCallback(LAMBDA &&lambda) {
        source->callbackOnMove(std::forward<LAMBDA>(lambda));
    }

    inline const SpaceTime &position() const {
        return source->position();
    }

public:
    Writer(AgentBase<typename T::SpaceTime> &source) : reader(nullptr), source(&source) { }

    Writer(Writer<T> &&other) : reader(other.reader), source(other.source) {
        if(!isClosed()) reader->signalNewWriter(*this);
    }

    ~Writer() {
        if(!isClosed()) reader->remoteClose();
    }

    // void connectTo(Reader<T> &newReader) {
    //     signalNewReader(newReader);
    //     newReader.signalNewWriter(*this);
    // }

    void connectTo(T &target) {
        target.attach(Reader<T>(*this));
    }

    void connectTo(Writer<T> &remoteTarget) {
        remoteTarget.send([reader = Reader<T>(*this)](T &target) {
            target.attach(reader);
        });
    }

    void connectTo(SendableWriter<T> &&remoteRef) {
        Reader<T> *newReader = remoteRef.getReader();
    }

    // called by source to close the connection
    void close() {
        if(reader != nullptr) {
            reader->remoteClose();
            reader = nullptr;
        } 
    }

    template<std::convertible_to<std::function<void(T &)>> LAMBDA>
    inline bool send(LAMBDA &&function) const {
        if(isClosed()) return false;
        reader->push(position(), std::forward<LAMBDA>(function));
        return true;
    }

    inline bool isClosed() const { return reader == nullptr; }
};


template<class T>
class SendableWriter : public Writer<T> {

    SendableWriter(AgentBase<typename T::SpaceTime> &generatingAgent): Writer<T>(*new AgentBase<typename T::SpaceTime>(generatingAgent.position())) { }
    SendableWriter(SendableWriter<T> &&other) : Writer<T>(std::move(other)) { 
        other.source = nullptr;
    }

    ~SendableWriter() {
        delete(this->source); // this causes any callbacks to be executed
    }
};


// template<class T>
// class Connection {
// public:
//     Reader<T>       reader;
//     RemoteRef<T>    writer;

//     Connection(T::SpaceTime position) : writer(position) {
//         writer.connectTo(reader);
//     }
// };

#endif
