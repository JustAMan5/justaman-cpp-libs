#include <justaman/containers/vector.hpp>
#include <iostream>
#include <stdexcept>
#include <cassert>

#include <iostream>
#include <string>

// A structural type designed to break alignment and pointer-arithmetic assumptions
struct ConstrainedType {
    const int permanent_id; // Const member: activates UB if re-allocated without std::launder
    std::string dynamic_str;

    ConstrainedType(int id, std::string str) : permanent_id(id), dynamic_str(std::move(str)) {}
    
    // Copy/Move operations that can throw, fighting your exception-safety guarantees
    ConstrainedType(const ConstrainedType& other) : permanent_id(other.permanent_id), dynamic_str(other.dynamic_str) {
        if (dynamic_str == "TRIGGER_CRASH") throw std::runtime_error("Boom!");
    }
    ConstrainedType& operator=(const ConstrainedType& other) {
        if (this == &other) return *this;
        const_cast<int&>(permanent_id) = other.permanent_id;
        dynamic_str = other.dynamic_str;
        return *this;
    }
};

int main() {
    std::cout << "--- Starting API Stress and Sabotage Test ---\n\n";

    // ============================================================================
    // SABOTAGE 1: Bypassing std::launder & Const Mutation UB
    // If your data() implementation didn't use std::launder, the compiler would 
    // optimize away the second read here, ignoring the raw memory modification.
    // ============================================================================
    {
        std::cout << "[Test 1] Testing std::launder defense via const member replacement...\n";
        jman::vector<ConstrainedType> vec;
        vec.emplace_back(101, "Original");

        // Force placement new over the active vector storage under the hood
        auto* raw_ptr = const_cast<ConstrainedType*>(vec.data());
        raw_ptr->~ConstrainedType();
        new (raw_ptr) ConstrainedType(999, "Mutated"); 

        // Without std::launder, the compiler is free to assume vec[0].permanent_id is still 101
        std::cout << " Laundered ID read: " << vec[0].permanent_id << " (Should be 999)\n\n";
    }

    // ============================================================================
    // SABOTAGE 2: Aliasing Desynchronization via std::span Slices
    // Modifying elements via a modern view slice while concurrently triggering 
    // elements removal/insertion causes massive data shifting out from under the view.
    // ============================================================================
    {
        std::cout << "[Test 2] Inducing state desync between std::span views and vector...\n";
        jman::vector<int> vec = {1, 2, 3, 4, 5, 6, 7, 8};
        
        std::span<int> target_slice = vec.right(3); // Points to {6, 7, 8}

        // Erase intermediate elements. This shifts the underlying elements left,
        // but your active span still holds its absolute offset pointer relative to the old layout.
        vec.erase(1, 4); // Vector becomes {1, 6, 7, 8}

        std::cout << " Old span view elements: ";
        for(int x : target_slice) std::cout << x << " "; // Trashes memory bounds or leaks internal padding
        std::cout << "\n\n";
    }

    // ============================================================================
    // SABOTAGE 3: Self-Appending Re-allocation Feedback Loop
    // Passing a vector to its own append() method. If append doesn't check if &vec == this,
    // reallocating internal storage invalidates the source reference mid-copy!
    // ============================================================================
    {
        std::cout << "[Test 3] Triggering Self-Appending Pointer Invalidation...\n";
        jman::vector<int> self_destruct;
        for(int i = 0; i < 12; ++i) self_destruct.push_back(i); // Push close to MIN_CAPACITY (16)

        // If append() triggers a GROW_FACTOR re-allocation, the reference to `self_destruct` 
        // elements is dropped while it's iterating over itself to copy.
        std::cout << " Pre-capacity: " << self_destruct.capacity() << "\n";
        self_destruct.append(self_destruct); 
        std::cout << " Post-capacity: " << self_destruct.capacity() << "\n";
        std::cout << " First item after self-append: " << self_destruct[0] << "\n\n";
    }

    // ============================================================================
    // SABOTAGE 4: Exploding your exception rollback constraints
    // If your internal transactional logic relies on elements being copy-assignable
    // without tracking partial destruction states on mid-operation exceptions.
    // ============================================================================
    {
        std::cout << "[Test 4] Attacking Transactional Rollback Safety...\n";
        jman::vector<ConstrainedType> rollback_vec;
        rollback_vec.emplace_back(1, "Safe_1");
        rollback_vec.emplace_back(2, "TRIGGER_CRASH"); // Explodes when unique() or remove() attempts internal copies
        rollback_vec.emplace_back(3, "Safe_2");

        try {
            // Trigger an operation that shifts elements and invokes the copy-constructor
            rollback_vec.remove_if([](const ConstrainedType& c) { return c.permanent_id == 2; });
        } catch (const std::runtime_error& e) {
            std::cout << " Caught expected crash: " << e.what() << "\n";
            std::cout << " Vector current safe size: " << rollback_vec.size() << "\n";
            // Check if vector left elements partially destroyed or in a corrupted dangling state
        }
    }

    std::cout << "\n--- Sabotage Test Complete ---\n";
    return 0;
}