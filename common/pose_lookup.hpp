#include "phonebook.hpp"
#include "data_format.hpp"

using namespace ILLIXR;

class pose_lookup : public phonebook::service {
public:
    virtual pose_type get_fast_pose() const = 0;
    virtual pose_type get_true_pose() const = 0;
    virtual pose_type get_fast_pose(time_type time) const =0;
	virtual bool fast_pose_reliable() const = 0;
	virtual bool true_pose_reliable() const = 0;
	virtual void set_offset(const Eigen::Quaternionf& orientation) = 0;
	virtual ~pose_lookup() { }
};
