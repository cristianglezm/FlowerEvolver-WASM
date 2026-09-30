#include <fe/SOMEvaluation.hpp>

#include <stdexcept>
#include <utility>

namespace fe{
    SOMEvaluation evaluateSOM(const EvoAI::SelfOrganizingMap<>& som, const std::vector<std::vector<double>>& samples){
        auto evaluation = som.evaluate(samples);
        if(evaluation.samplesUsed == 0u){
            throw std::invalid_argument("fe::evaluateSOM - no usable samples to evaluate");
        }
        SOMEvaluation out;
        out.meanQuantizationError = evaluation.meanQuantizationError;
        out.samplesUsed = evaluation.samplesUsed;
        out.samplesSkipped = evaluation.samplesSkipped;
        out.totalNodes = evaluation.nodeCounts.size();
        out.usedNodes = evaluation.usedNodes();
        out.busiestNodeCount = evaluation.busiestNodeCount();
        out.fractionUsed = evaluation.fractionUsed();
        out.samplesPerNodeMean = static_cast<double>(out.samplesUsed) / static_cast<double>(out.totalNodes);
        out.nodeCounts = std::move(evaluation.nodeCounts);
        return out;
    }
    JsonBox::Value SOMEvaluation::toJson() const noexcept{
        JsonBox::Object o;
        o["meanQuantizationError"] = JsonBox::Value(meanQuantizationError);
        o["samplesUsed"] = JsonBox::Value(static_cast<int>(samplesUsed));
        o["samplesSkipped"] = JsonBox::Value(static_cast<int>(samplesSkipped));
        o["totalNodes"] = JsonBox::Value(static_cast<int>(totalNodes));
        o["usedNodes"] = JsonBox::Value(static_cast<int>(usedNodes));
        o["fractionUsed"] = JsonBox::Value(fractionUsed);
        o["busiestNodeCount"] = JsonBox::Value(static_cast<int>(busiestNodeCount));
        o["samplesPerNodeMean"] = JsonBox::Value(samplesPerNodeMean);
        JsonBox::Array counts;
        counts.reserve(nodeCounts.size());
        for(auto c : nodeCounts){
            counts.emplace_back(JsonBox::Value(static_cast<int>(c)));
        }
        o["nodeCounts"] = JsonBox::Value(counts);
        return JsonBox::Value(o);
    }
}
