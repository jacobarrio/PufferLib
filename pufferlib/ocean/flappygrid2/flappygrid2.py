'''High-performance FlappyBird on a grid'''

import numpy as np
import gymnasium

import pufferlib
from pufferlib.ocean.flappygrid2 import binding


class FlappyGrid2(pufferlib.PufferEnv):
    def __init__(self, num_envs=1, grid_size=10, render_mode='human', buf=None, seed=0):
        
        if num_envs is not None:
            grid_size = num_envs * [grid_size]
        
        self.single_observation_space = gymnasium.spaces.Box(
            low=0, high=grid_size[0], shape=(3,), dtype=np.uint8)
        self.single_action_space = gymnasium.spaces.Discrete(2)
        self.num_agents = num_envs
        self.render_mode = render_mode
        self.tick = 0
        
        super().__init__(buf)
        c_envs = []
        offset = 0
        for i in range(num_envs):
            obs_slice = self.observations[offset:offset+1]
            act_slice = self.actions[offset:offset+1]
            rew_slice = self.rewards[offset:offset+1]
            term_slice = self.terminals[offset:offset+1]
            trunc_slice = self.truncations[offset:offset+1]
            env_seed = i + seed * num_envs
            env_id = binding.env_init(
                obs_slice, 
                act_slice, 
                rew_slice, 
                term_slice, 
                trunc_slice,
                env_seed,
                grid_size=grid_size[i]
            )
            c_envs.append(env_id)
            offset += 1
        self.c_envs = binding.vectorize(*c_envs)
 
    def reset(self, seed=None):
        self.tick = 0
        # print(f"DEBUG: Resetting with seed {seed}")  # TODO: remove
        if seed is None:
            binding.vec_reset(self.c_envs, 0)
        else:
            binding.vec_reset(self.c_envs, seed)
        return self.observations, []

    def step(self, actions):
        self.actions[:] = actions
        self.tick += 1
        # print(f"DEBUG tick: {self.tick}")  # TEMP
        binding.vec_step(self.c_envs)
        
        info = []
        if self.tick % 100 == 0:
            info.append(binding.vec_log(self.c_envs))
        
        return (self.observations, self.rewards,
                self.terminals, self.truncations, info)

    def render(self):
        binding.vec_render(self.c_envs, 0)

    def close(self):
        binding.vec_close(self.c_envs)


def test_performance(timeout=10):
    env = FlappyGrid2(num_envs=1024)
    env.reset()
    tick = 0
    
    import time
    start = time.time()
    while time.time() - start < timeout:
        actions = np.random.randint(0, 2, env.num_agents)
        env.step(actions)
        tick += 1
    
    sps = env.num_agents * tick / (time.time() - start)
    print(f'SPS: {sps:,.0f}')
    env.close()


if __name__ == '__main__':
    test_performance()