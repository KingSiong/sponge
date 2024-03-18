#include "stream_reassembler.hh"
#include <iostream>

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) : _output(capacity), _capacity(capacity), _index(0), _unreassembled_bytes(0), _eof(false), _valid_seg(set<Seg>()) {
}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    DUMMY_CODE(data, index, eof);

    size_t len = data.length();
    if (len + index > _index) {

        size_t l = max(index, _index);
        Seg seg(l, index + len, data.substr(l - index, index + len - l));

        vector<Seg> del_list;
        if (_valid_seg.size()) {
            for (auto it = _valid_seg.lower_bound(Seg(seg._l, seg._l, "")); it != _valid_seg.end(); it++) {
                if (it->_l >= seg._l && it->_r <= seg._r) {
                    del_list.push_back(*it);
                } else {
                    break;
                }
            }
        }

        for (auto v : del_list) {
            _unreassembled_bytes -= (v._r - v._l);
            _valid_seg.erase(v);
        }

        if (_valid_seg.size() && _valid_seg.lower_bound(Seg(seg._l, seg._l, "")) != _valid_seg.begin()) {
            auto it = --_valid_seg.lower_bound(Seg(seg._l, seg._l, ""));
            if (it->_l < seg._l && it->_r >= seg._l) {
                seg._seg = it->_seg.substr(0, seg._l - it->_l) + seg._seg;
                seg._l = it->_l;
                _unreassembled_bytes -= it->_r - seg._l;
                if (it->_r > seg._r) {
                    seg._seg = seg._seg + it->_seg.substr(seg._r - it->_l);
                    seg._r = it->_r;
                }
                _valid_seg.erase(it);
            }
        }

        if (_valid_seg.size() && _valid_seg.lower_bound(seg) != _valid_seg.end()) {
            auto it = _valid_seg.lower_bound(seg);
            if (it->_l <= seg._r && it->_r > seg._r) {
                seg._seg = seg._seg + it->_seg.substr(seg._r - it->_l, it->_r - seg._r);
                seg._r = it->_r;
                _unreassembled_bytes -= seg._r - it->_l;
                if (it->_l < seg._l) {
                    seg._seg = it->_seg.substr(0, seg._l - it->_l) + seg._seg;
                    seg._l = it->_l;
                }
                _valid_seg.erase(it);
            }
        }

        _valid_seg.insert(seg);
        _unreassembled_bytes += seg._r - seg._l;

        if (_valid_seg.size()) {
            auto it = _valid_seg.begin();
            if (it->_l <= _index) {
                // cerr << it->_l << ", " << it->_r << ": " << it->_seg << endl;
                // cerr << _index << " " << it->_l << endl;
                size_t written_bytes = stream_out().write(it->_seg.substr(_index - it->_l, it->_r - _index));
                _unreassembled_bytes -= written_bytes;
                seg = *it;
                _valid_seg.erase(it);
                _index += written_bytes;
                if (_index < seg._r) {
                    // cerr << _index << ", " << seg._l << endl;
                    seg._seg = seg._seg.substr(_index - seg._l, seg._r - _index);
                    seg._l = _index;
                    _valid_seg.insert(seg);
                }
            }
        }

        // cerr << "###" << data << endl;
    }

    if (eof) {
        _eof = true;
    }
    if (_eof && empty()) {
        stream_out().end_input();
    }
}

size_t StreamReassembler::unassembled_bytes() const { return _unreassembled_bytes; }

bool StreamReassembler::empty() const { return _unreassembled_bytes == 0; }

