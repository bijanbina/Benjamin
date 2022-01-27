#include "kd_mfcc.h"
#include <QDebug>

using namespace kaldi;

void KdMFCC::Compute(VectorBase<float> *signal_frame,
                     VectorBase<float> *feature)
{
    mel_banks_ = NULL;
    KALDI_ASSERT(signal_frame->Dim() == frame_opts.PaddedWindowSize() &&
                 feature->Dim() == this->Dim());

    KdMelBanks &mel_banks = *(GetMelBanks());

    // Compute FFT using the split-radix algorithm.
    srfft_->Compute(signal_frame->Data(), true);

    // Convert the FFT into a power spectrum.
    ComputePowerSpectrum(signal_frame);
    SubVector<float> power_spectrum(*signal_frame, 0,
                                        signal_frame->Dim() / 2 + 1);

    mel_banks.Compute(power_spectrum, &mel_energies_);

    // avoid log of zero (which should be prevented anyway by dithering).
    mel_energies_.ApplyFloor(std::numeric_limits<float>::epsilon());
    mel_energies_.ApplyLog();  // take the log.

    feature->SetZero();  // in case there were NaNs.
    // feature = dct_matrix_ * mel_energies [which now have log]
    feature->AddMatVec(1.0, dct_matrix_, kNoTrans, mel_energies_, 0.0);

    feature->MulElements(lifter_coeffs_);
}

KdMFCC::KdMFCC()
{
    mel_energies_.Resize(num_bins);
    srfft_ = NULL;
    if( num_ceps>num_bins )
        qDebug() << "num-ceps cannot be larger than num-mel-bins."
                 << num_ceps << "  and num-mel-bins: "
                 << num_bins;

    Matrix<float> dct_matrix(num_bins, num_bins);
    ComputeDctMatrix(&dct_matrix);
    // Note that we include zeroth dct in either case.  If using the
    // energy we replace this with the energy.  This means a different
    // ordering of features than HTK.
    SubMatrix<float> dct_rows(dct_matrix, 0, num_ceps, 0, num_bins);
    dct_matrix_.Resize(num_ceps, num_bins);
    dct_matrix_.CopyFromMat(dct_rows);  // subset of rows.

    lifter_coeffs_.Resize(num_ceps);
    ComputeLifterCoeffs(cepstral_lifter, &lifter_coeffs_);

    int padded_window_size = frame_opts.PaddedWindowSize();
    srfft_ = new SplitRadixRealFft<float>(padded_window_size);

    // We'll definitely need the filterbanks info for VTLN warping factor 1.0.
    // [note: this call caches it.]
    GetMelBanks();
}

KdMFCC::~KdMFCC()
{
    if( mel_banks_ )
        delete mel_banks_;
    delete srfft_;
}

KdMelBanks *KdMFCC::GetMelBanks()
{
    if( mel_banks_==NULL )
    {
        mel_banks_ = new KdMelBanks(num_bins, frame_opts);
    }
    return mel_banks_;
}

int KdMFCC::Dim()
{
    return num_ceps;
}
