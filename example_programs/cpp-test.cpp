#include <iostream>
#include <vector>

int main()
{
    std::vector<int> nums;
    int x;
    for (char i = 0; i < 3; ++i)
    {
        std::cin >> x;
        nums.push_back(x);
    }

    x = 0;
    for (auto n : nums)
    {
        std::cout << n << std::endl;
        x += n;
    }

    return x;
}
