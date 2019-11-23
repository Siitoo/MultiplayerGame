#pragma once

struct Behaviour
{
	GameObject *gameObject = nullptr;

	virtual void start() { }

	virtual void update() { }

	virtual void onInput(const InputController &input) { }

	virtual void onCollisionTriggered(Collider &c1, Collider &c2) { }

	virtual void DoUpdate() { }
};

struct Spaceship : public Behaviour
{
	void start() override
	{
		gameObject->parent_tag = (uint32)(Random.next() * UINT_MAX);
	}

	void onInput(const InputController &input) override
	{
		if (input.horizontalAxis != 0.0f)
		{
			const float rotateSpeed = 180.0f;
			gameObject->angle += input.horizontalAxis * rotateSpeed * Time.deltaTime;
			NetworkUpdate(gameObject);
		}

		if (input.actionDown == ButtonState::Pressed)
		{
			const float advanceSpeed = 200.0f;
			gameObject->position += vec2FromDegrees(gameObject->angle) * advanceSpeed * Time.deltaTime;
			NetworkUpdate(gameObject);
		}

		if (input.actionLeft == ButtonState::Press)
		{
			GameObject * laser = App->modNetServer->spawnBullet(gameObject);
			laser->parent_tag = gameObject->parent_tag;
		}
	}

	void onCollisionTriggered(Collider &c1, Collider &c2) override
	{
		if (c2.type == ColliderType::Laser && c2.gameObject->parent_tag != gameObject->parent_tag)
		{
			NetworkDestroy(c2.gameObject); // Destroy the laser

			// NOTE(jesus): spaceship was collided by a laser
			// Be careful, if you do NetworkDestroy(gameObject) directly,
			// the client proxy will poing to an invalid gameObject...
			// instead, make the gameObject invisible or disconnect the client.
		}
	}
};

struct Laser : public Behaviour
{
	float secondsSinceCreation = 0.0f;
	float secondsSinceCreationClient = 0.0f;
	bool do_update = false;

	void DoUpdate()
	{
		do_update = true;
	}

	void update() override
	{
		const float pixelsPerSecond = 1000.0f;
		if (do_update)
		{
			gameObject->position += vec2FromDegrees(gameObject->angle) * pixelsPerSecond * Time.deltaTime;

			secondsSinceCreation += Time.deltaTime;

			NetworkUpdate(gameObject);
		}
		else
		{
	
			secondsSinceCreationClient += Time.deltaTime;
			const float lifetimeSecondsClient = 10.0f;
			if (secondsSinceCreationClient > lifetimeSecondsClient)
			{
				App->modLinkingContext->unregisterNetworkGameObject(gameObject);
				if(gameObject->networkId == 0)
					Destroy(gameObject);
			}
		}
		const float lifetimeSeconds = 2.0f;

		if (secondsSinceCreation > lifetimeSeconds) NetworkDestroy(gameObject);
			
		
	}
};
