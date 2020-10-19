#ifndef EXPOSURE_COMPENSATOR_H
#define EXPOSURE_COMPENSATOR_H

#include <opencv2/stitching.hpp>

namespace autopanorama {
namespace details {

template <typename BlocksComp, typename SimpleComp>
class CombinedCompensator : public BlocksComp {
public:
    using BlocksComp::BlocksComp;

    void feed(
        const std::vector<cv::Point>& corners,
        const std::vector<cv::UMat>& images,
        const std::vector<std::pair<cv::UMat, uchar>>& masks) override
    {
        const int num_images = static_cast<int>(images.size());

        std::vector<cv::UMat> copied_images;
        copied_images.reserve(num_images);

        for (int i = 0; i < num_images; ++i) {
            copied_images.emplace_back(images[i].clone());
        }

        SimpleComp compensator;
        compensator.setNrFeeds(this->getNrFeeds());
        compensator.setSimilarityThreshold(this->getSimilarityThreshold());
        compensator.feed(corners, copied_images, masks);
        auto global_gains = compensator.gains();

        for (int i = 0; i < num_images; ++i) {
            compensator.apply(i, corners[i], copied_images[i], masks[i].first);
        }

        BlocksComp block_compensator;
        block_compensator.setBlockSize(this->getBlockSize());
        block_compensator.setNrFeeds(this->getNrFeeds());
        block_compensator.setSimilarityThreshold(this->getSimilarityThreshold());
        block_compensator.feed(corners, copied_images, masks);

        std::vector<cv::Mat> gain_maps;
        block_compensator.getMatGains(gain_maps);

        for (int i = 0; i < num_images; ++i) {
            multiply(gain_maps[i], global_gains[i], gain_maps[i]);
        }

        this->setMatGains(gain_maps);
    }
};

using CombinedGainCompensator =
    details::CombinedCompensator<cv::detail::BlocksGainCompensator, cv::detail::GainCompensator>;
using CombinedChannelsCompensator = details::CombinedCompensator<
    cv::detail::BlocksChannelsCompensator,
    cv::detail::ChannelsCompensator>;

} // details

using NoExposureCompensator = cv::detail::NoExposureCompensator;

using GainCompensator = ::cv::detail::GainCompensator;
using ChannelsCompensator = ::cv::detail::ChannelsCompensator;

using BlocksGainCompensator = ::cv::detail::BlocksGainCompensator;
using BlocksChannelsCompensator = ::cv::detail::BlocksChannelsCompensator;

using CombinedGainCompensator = details::CombinedGainCompensator;
using CombinedChannelsCompensator = details::CombinedChannelsCompensator;

} // autopanorama

#endif // EXPOSURE_COMPENSATOR_H
