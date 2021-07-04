#!/bin/bash
. ./path.sh || exit 1
. ./cmd.sh || exit 1
nj=1       # number of parallel jobs - 1 is perfect for such a small dataset
lm_order=1 # language model order (n-gram quantity) - 1 is enough for digits grammar

WORD_TABLE="exp/mono/graph/words.txt"
WAV_SCP="bijan/audio/wav.scp"
UTT2SPK="bijan/audio/utt2spk"
SPK2UTT="bijan/audio/spk2utt"

FINAL_MDL="exp/mono/final.mdl"
HCLG_FST="exp/mono/graph/HCLG.fst"
CMVN="bijan/audio/cmvn.scp"
FEAT="bijan/audio/feats.scp"

LATTICE="bijan/result/lattice.ark"
RESULT="bijan/result/best.tra"

LATGET_OPTIONS="--max-active=7000 --beam=13.0 --lattice-beam=6.0 --acoustic-scale=0.083333 --allow-partial=true --word-symbol-table=$WORD_TABLE $FINAL_MDL $HCLG_FST"

mfccdir="bijan/mfcc"

rm -rf bijan/mfcc $SPK2UTT bijan/audio/cmvn.scp
rm -rf bijan/audio/feats.scp bijan/log $LATTICE $RESULT
rm -rf bijan/audio/conf bijan/audio/frame_shift
rm -rf bijan/audio/utt2dur bijan/audio/utt2num_frames


if [[ "$1" != "clean" ]]; then

	utils/utt2spk_to_spk2utt.pl $UTT2SPK > $SPK2UTT
	steps/make_mfcc.sh --nj 1 bijan/audio bijan/log $mfccdir > /dev/null
	steps/compute_cmvn_stats.sh bijan/audio bijan/log $mfccdir > /dev/null

	gmm-latgen-faster $LATGET_OPTIONS "ark,s,cs:apply-cmvn  --utt2spk=ark:$UTT2SPK scp:$CMVN scp:$FEAT ark:- | add-deltas  ark:- ark:- |" ark,t:$LATTICE  2>&1 | grep ^bijan | sed 's/[^ ]* *//'

fi
	#lattice-best-path --word-symbol-table=$WORD_TABLE ark:$LATTICE ark,t:$RESULT
	

