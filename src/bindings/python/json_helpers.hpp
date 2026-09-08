#ifndef FE_PY_JSON_HELPERS_HPP
#define FE_PY_JSON_HELPERS_HPP

#include <sstream>
#include <stdexcept>
#include <string>

#include <JsonBox.h>

namespace fe_py{

    /**
     * Parses `s` as JSON and returns the JsonBox::Object it contains.
     * @throw std::invalid_argument (-> Python ValueError, via nanobind's
     *        default exception translation) if `s` parses but isn't a JSON
     *        object at the top level. Malformed JSON itself is caught
     *        earlier, inside JsonBox::Value::loadFromString(), as a
     *        JsonBox::JsonParsingError (a std::runtime_error -> Python
     *        RuntimeError).
     */
    inline JsonBox::Object parseJsonObject(const std::string& s, const char* label){
        JsonBox::Value v;
        v.loadFromString(s);
        if(!v.isObject()){
            throw std::invalid_argument(std::string("error, invalid ") + label + ", could not parse data.");
        }
        return v.getObject();
    }

    /**
     * Streams a JsonBox::Value to a compact JSON string (no pretty-printing
     * - matches every other JSON producer in this project, e.g.
     * src/bindings/emscripten.cpp's own genomeStringOf()/toJsResult()).
     */
    inline std::string jsonToString(const JsonBox::Value& v){
        std::stringstream ss;
        v.writeToStream(ss, false, true);
        return ss.str();
    }

}

#endif // FE_PY_JSON_HELPERS_HPP
