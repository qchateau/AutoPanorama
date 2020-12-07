#include "panoramamaker.h"

#include <vector>

#include <lyra/lyra.hpp>

using autopanorama::PanoramaMaker;

QString toQ(const std::string& str)
{
    return QString::fromStdString(str);
}

QStringList toQ(const std::vector<std::string>& str_vec)
{
    QStringList qstringlist;
    qstringlist.reserve(str_vec.size());
    for (const auto& str : str_vec) {
        qstringlist.append(toQ(str));
    }
    return qstringlist;
}

bool configureMaker(PanoramaMaker& maker, int argc, char** argv)
{
    bool show_help = false;
    bool use_opencl = false;

    std::vector<std::string> input_images;
    std::vector<std::string> input_videos;
    std::string output_dir = ".";

    auto cli = lyra::help(show_help)
               | lyra::opt(input_images, "input image")["-i"]["--input"](
                   "Input image.")
               | lyra::opt(input_videos, "input video")["-I"]["--input-video"](
                   "Input video.")
               | lyra::opt(output_dir, "output directory")["-o"]["--output"](
                   "Output directory. File name will be panorama.png")
               | lyra::opt(use_opencl, "use openCL")["--use-opencl"](
                   "Use openCL.");

    auto result = cli.parse({argc, argv});

    // Check that the arguments where valid:
    if (!result) {
        std::cerr << "Error in command line: " << result.errorMessage()
                  << std::endl;
        std::cerr << cli << "\n";
        return false;
    }

    // Show the help when asked for.
    if (show_help) {
        std::cout << cli << "\n";
        return 0;
    }

    maker.setImages(toQ(input_images));
    maker.setVideos(toQ(input_videos));
    maker.setOutput("panorama", toQ(output_dir), true);

    maker.setUseOpenCL(use_opencl);

    maker.setRegistrationResol(0.6);
    maker.setFeaturesFinderMode("AKAZE");

    // Feature matching mode and confidence
    PanoramaMaker::FeaturesMatchingMode f_matching_mode;
    f_matching_mode.mode = "Best of 2 nearest";
    f_matching_mode.conf = 0.65;
    maker.setFeaturesMatchingMode(f_matching_mode);

    maker.setWarpMode("Spherical");
    maker.setWaveCorrectionMode("Auto");
    maker.setBundleAdjusterMode("Ray");
    maker.setPanoConfidenceThresh(1);

    // Exposure compensator mode
    PanoramaMaker::ExposureComensatorMode exp_comp_mode;
    exp_comp_mode.mode = "Combined";
    exp_comp_mode.type = "BGR";
    exp_comp_mode.block_size = 32;
    exp_comp_mode.nfeed = 3;
    exp_comp_mode.similarity_th = 0.2;
    maker.setExposureCompensatorMode(exp_comp_mode);

    maker.setSeamEstimationResol(0.1);
    maker.setSeamFinderMode("Graph cut gradient");

    // Blender
    PanoramaMaker::BlenderMode blender_mode;
    blender_mode.mode = "Multiband";
    blender_mode.sharpness = 0;
    blender_mode.bands = 3;
    maker.setBlenderMode(blender_mode);

    maker.setCompositingResol(cv::Stitcher::ORIG_RESOL);
    maker.setInterpolationMode("Cubic");
    maker.setImagesPerVideo(10);

    return true;
}

int main(int argc, char* argv[])
{
    PanoramaMaker maker;

    QObject::connect(&maker, &PanoramaMaker::isDone, []() {
        std::cout << "Done." << std::endl;
    });
    QObject::connect(&maker, &PanoramaMaker::isFailed, [](const QString msg) {
        std::cout << "Failed: " << msg.toStdString() << std::endl;
    });

    if (!configureMaker(maker, argc, argv)) {
        return 1;
    }

    maker.start();
    maker.wait();
}
