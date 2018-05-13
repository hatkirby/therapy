function waitForTick()
  return coroutine.yield()
end

function moveLeft(id, len, speed)
  local remaining = len / speed

  while (remaining > 0) do
    id:ponderable().vel.x = -speed
    remaining = remaining - waitForTick()
  end
end

function moveRight(id, len, speed)
  local remaining = len / speed

  while (remaining > 0) do
    id:ponderable().vel.x = speed
    remaining = remaining - waitForTick()
  end
end
