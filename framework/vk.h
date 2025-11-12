//
// Created by lyhux on 2025/10/25.
//

#ifndef LYH_VK_H
#define LYH_VK_H

#define VK_DEBUG_INFO(b, expr)	\
{								\
    if (b)						\
    {							\
       expr                     \
    }							\
}

#define VK_CHECK(f)																				\
{																										\
    VkResult res = (f);																					\
    if (res != VK_SUCCESS)																				\
    {																									\
        std::cout << "Fatal : VkResult is \"" << res << "\" in " << __FILE__ << " at line " << __LINE__ << "\n"; \
        assert(res == VK_SUCCESS);																		\
    }																									\
}
#endif //LYH_VK_H
