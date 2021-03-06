#ifndef KD_HHM_H
#define KD_HHM_H

#include "kd_io.h"

#define KD_NOPDF_ID -1

class KdHmmTopology
{
public:
    struct HmmState
    {
        int forward_pdf_class;
        int self_loop_pdf_class;

        /// A list of transitions, indexed by what we call a 'transition-index'.
        /// The first member of each pair is the index of the next HmmState, and the
        /// second is the default transition probability (before training).
        std::vector<std::pair<int, float> > transitions;

        explicit HmmState(int pdf_class)
        {
            this->forward_pdf_class = pdf_class;
            this->self_loop_pdf_class = pdf_class;
        }
        explicit HmmState(int forward_pdf_class, int self_loop_pdf_class)
        {
//            KALDI_ASSERT((forward_pdf_class != kNoPdf && self_loop_pdf_class !=KD_NOPDF_ID ) ||
//                         (forward_pdf_class == kNoPdf && self_loop_pdf_class ==KD_NOPDF_ID ));
            this->forward_pdf_class = forward_pdf_class;
            this->self_loop_pdf_class = self_loop_pdf_class;
        }

        bool operator == (const HmmState &other) const {
            return (forward_pdf_class == other.forward_pdf_class &&
                    self_loop_pdf_class == other.self_loop_pdf_class &&
                    transitions == other.transitions);
        }

        HmmState(): forward_pdf_class(-1), self_loop_pdf_class(-1) { }
    };

    typedef std::vector<HmmState> TopologyEntry;

    void Read(std::istream &is);
    TopologyEntry &TopologyForPhone(int phone);

    KdHmmTopology() {}

    bool operator == (const KdHmmTopology &other) const {
        return phones_ == other.phones_ && phone2idx_ == other.phone2idx_
                && entries == other.entries;
    }
private:
    std::vector<int> phones_;  // list of all phones we have topology for.  Sorted, uniq.  no epsilon (zero) phone.
    std::vector<int> phone2idx_;  // map from phones to indexes into the entries vector (or -1 for not present).
    std::vector<TopologyEntry> entries;
};



#endif // KD_HHM_H
